#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include "qdbmp.h"
#include "julia.h"
#include "mpi.h"



MPI_Status status;
unsigned int data[HEIGHT * WIDTH];
unsigned int hist[MAX_ITER] = {0};
// Takes an integer colour value and splits it into its RGB component parts.
// val (a 32-bit unsigned integer type) is expected to contain a 24-bit unsigned integer.
void toRGB(unsigned int val,
           unsigned char *r, unsigned char *g, unsigned char *b)
{
    // intentionally mixed up the order here to make the colours a little nicer...
    *r = (val & 0xff);
    val >>= 8;
    *b = (val & 0xff);
    val >>= 8;
    *g = (val & 0xff);
}

// Returns the sum of the elements in the given array.
unsigned int sum_array(unsigned int *array, int len)
{
    unsigned int total = 0;
    for (int i = 0; i < len; i++)
    {
        total += array[i];
    }

    return total;
}

// Perform "histogram colour equalization" on the data array, using the
// information in the histogram array.
// This just ensures that the colours get nicely distributed to different
// values in the data array (i.e. makes sure that if the data array only contains values
// in a narrow range (between 100 and 200), the colours won't all be the same.
void hist_eq(unsigned int *data, unsigned int *hist)
{
    unsigned int total = sum_array(hist, MAX_ITER);
    unsigned int val;

    // Create a cache to speed up the loops below,
    // since they'll require the use of the same values many times
    float cache[MAX_ITER];
    float hue = 0.0;
    for (unsigned int i = 0; i < MAX_ITER; i++)
    {
        cache[i] = hue;
        hue += (float) hist[i] / total;
    }

    // Go through each pixel in the output image and tweak its colour value
    // (such that when we're done, the colour values in the data array have a uniform distribution)
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            val = data[y * WIDTH + x];

            // if the number's cached, use it
            if (val < MAX_ITER)
            {
                hue = cache[val];
            }
            //otherwise, calculate it
            else
            {
                hue = cache[MAX_ITER - 1];
                for (unsigned int i = MAX_ITER; i < val; i++)
                {
                    hue += (float) hist[i] / total;
                }
            }

            // expand the value's range from [0, 1] to [0, MAX_COLOUR]
            data[y * WIDTH + x] = (unsigned int) (hue * MAX_COLOUR);
        }
    }
}

// Writes the given data to a bitmap (.bmp) file with the given name.
// To do this, it interprets each value in the data array as an RGB colour
// (by calling toRGB()).
void write_bmp(unsigned int *data, char *fname)
{
    BMP *bmp = BMP_Create((UINT) WIDTH, (UINT) HEIGHT, (USHORT) DEPTH);
    unsigned char r, g, b;
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            toRGB(data[y * WIDTH + x], &r, &g, &b);
            BMP_SetPixelRGB(bmp, (UINT) x, (UINT) y,
                            (UCHAR) r, (UCHAR) g, (UCHAR) b);
        }
    }

    BMP_WriteFile(bmp, FNAME);
    BMP_Free(bmp);
}

// Generates terms of the Julia fractal sequence (starting with the given complex number)
// until either the imaginary part exceeds LIMIT or we hit MAX_ITER iterations.
unsigned int julia_iters(float complex z)
{
    unsigned int iter = 0;
    while (fabsf(cimag(z)) < LIMIT && iter < MAX_ITER)
    {
        z = C * csin(z);
        iter++; 
    }

    //this value will be used to colour a pixel on the screen
    return iter;
}

// Computes the colour data for one row of pixels in the output image.
// Results are stored in the data array. Also populates the histogram array
// with the counts of the distinct values in this row.

//void compute_row(int row, unsigned int *data, unsigned int *hist)
void compute_row(int row, unsigned int *data, unsigned int *hist)
{
    float complex z;
    float series_row;
    float series_col;
    unsigned int iters;
 
    for (int col = 0; col < WIDTH; col++)
    {
        series_row = row - HEIGHT / 2;
        series_col = col - WIDTH / 2;
        z = series_col / RES_FACTOR + (I / RES_FACTOR) * series_row;
        z *= SCALE;
        iters = julia_iters(z);
        data[row * WIDTH + col] = iters;
		//printf("%d\n\n", iters);
        hist[iters]++;
		
    }
}

int main(int argc, char *argv[])
{
	/*-----PARALLEL INTIALIZATION------*/
	int my_rank;
    int num_procs;
	int numworkers,source,rows,dest,offset,i,k;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //grab this process's rank
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs); //grab the total num of processes
	
	numworkers = num_procs-1; // Calculate the number of worker processes
	rows = HEIGHT/numworkers; // distribute rows among worker processes
	
	double start_time; // use these for timing
    double stop_time;
	
	/*---------------------------Manager Process ----------------------------*/
	if (my_rank==0)
	{
		//printf("---------------------------------------\n Process 0 \n\n\n");
		printf("Beginning julia set computation(Process 0)...\n\n");
		printf("Number of processes: %d\n", num_procs);
		
		start_time = MPI_Wtime(); // can use this function to grab a
                                  // timestamp (in seconds)
		offset = 0;
		for (dest=1; dest<=numworkers; dest++)//Shares the information and data among worker processes for parallel computation
		{
			MPI_Send(&offset, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, dest, 1, MPI_COMM_WORLD);
			offset = offset + rows;
		}
		
		for (i=1; i<=numworkers; i++)// Recieves the processed results from each worker processes and computes the output image. 
		{
		  unsigned int hste[MAX_ITER] = {0};
		  source = i;
		  MPI_Recv(&offset, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		  MPI_Recv(&rows, 1, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		  MPI_Recv(&data[offset*WIDTH], rows*WIDTH, MPI_INT, source, 2, MPI_COMM_WORLD, &status);
		  MPI_Recv(&hste,MAX_ITER,MPI_INT,source,2,MPI_COMM_WORLD,&status);
		  for (k=0;k<MAX_ITER;k++)
		  {
			  hist[k] = hist[k]+ hste[k];
		  }
		}
				
		//-----
		for(dest =1; dest<= numworkers; dest++)
		{
			MPI_Send(0,0, MPI_INT, dest, 2, MPI_COMM_WORLD);
		}
		//-----

		hist_eq(data, hist); // Computes histogram equalization
		write_bmp(data, FNAME); // write the output as image.
		
		stop_time = MPI_Wtime();
        
		printf("*****Computation Done*****\n");
		printf("Total execution time (sec): %f\n", stop_time - start_time);
	}
	
	/*-------------------Worker------------------------------------*/
	if (my_rank>0)
	{
		start_time = MPI_Wtime(); // can use this function to grab a
                                  // timestamp (in seconds)
		//printf(" *****Process %d computing****\n",my_rank);
		source = 0;
		MPI_Recv(&offset, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status); // recieve from Manager process(process 0)
		MPI_Recv(&rows, 1, MPI_INT, source, 1, MPI_COMM_WORLD, &status);// recieve from Manager process(process 0)
		// Local copy
		unsigned int dt[HEIGHT * WIDTH];
		unsigned int hst[MAX_ITER] = {0};
		
		for (int row = offset; row < (offset+rows); row++)
		{
			compute_row(row, dt, hst);//Compute for each row assigned to worker process
		}
		int count = 0;
		for (i = offset*WIDTH; i<(offset+rows)*WIDTH;i++)
		{
			//printf("%d",dt[i]);
			count++;
		}
		
		//printf("\n\n%d", sizeof(hst) / sizeof(int)); //Check size of Hist array
		MPI_Send(&offset, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&rows, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&dt[offset*WIDTH], rows*WIDTH, MPI_INT, 0, 2, MPI_COMM_WORLD);
		MPI_Send(&hst,MAX_ITER,MPI_INT,0,2,MPI_COMM_WORLD);
		//printf("\n---------------------------------------\n Process %d\n\n\n",my_rank);
		stop_time = MPI_Wtime();
		if(status.MPI_TAG== 2){
			return 0;					//Works as die-tag to tell slaves to quit, when all work is done 
			printf("complete");			
			}
		//printf("Process %d execution time time (sec): %f\n", my_rank,stop_time - start_time);
	}
	
	MPI_Finalize();
	return EXIT_SUCCESS;
} 


