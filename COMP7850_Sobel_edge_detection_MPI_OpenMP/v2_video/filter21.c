#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include "mpi.h"
#include <dirent.h>
  
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define CHANNEL_NUM 1

int main(int argc, char *argv[] ) {
    
    
	int nthreads = 0;
    
	//Sobel filter kernels
	int sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    //int sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
	int sobel_y[3][3] = { {1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
	
    int number_of_Processors = atoi(argv[3]);
    //int thread_id[number_threads];
	
	
	int ProcessorRank;	// Processor Rank
	int numnodes,start,end;
	double start_time; // use these for timing
	double stop_time;
	
	MPI_Init(&argc, &argv);					
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcessorRank); //get rank of node's process
	MPI_Comm_size(MPI_COMM_WORLD, &numnodes); // Number of Processors available
	
	
	//Root Process 
	if (ProcessorRank == 0) //if root process
	{      
		
		printf("No. of Processes: %d\n", number_of_Processors);
		//printf("Value of N : %d\n\n", n_iterations);
		start_time = MPI_Wtime(); // can use this function to grab a
                                  // timestamp (in seconds)
	}
	
	
	
///////////////////
// Get length code

DIR *dir;
struct dirent *ent;
int imges_ln = -2;
if ((dir = opendir ("tmp3")) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    //printf ("%s\n", ent->d_name);
	imges_ln+=1;
  }
  
  printf("\n Total image Frames : %d\n", imges_ln);
  closedir (dir);
} else {
  /* could not open directory */
  perror ("");
  return 0;
}

	numnodes = number_of_Processors;
	int ln_img = 144;
	int N = ln_img/numnodes;
	start = ProcessorRank*N;
	end = start + N;
	if (ProcessorRank == (numnodes-1)){
		start = ProcessorRank*N+1;
		int buffer = ln_img%numnodes;
		end = start + N + buffer -1;
	}
	else if(ProcessorRank == 0){
		start = ProcessorRank*N + 1;
		end = start + N -1;
	}
	else{
		start = ProcessorRank*N+1;
		end = start + N - 1;
	}

	
//uint8_t* images[143]={};
uint8_t* images[145]={};

//Reading all images
	int i = 1, kl;
	for(kl = start;kl<=end;kl++){
			int width, height, bpp;
			i = kl;
			char str[10];
			sprintf(str, "%d", kl);
			char text[100] = "tmp3/image";
			char tr[] = ".bmp";
			strcat(text,str);
			strcat(text,tr);
			//printf("%s", text); //prints A
			//text = "tmp3/image1.bmp"
			argv[1] = text;
			//images[kl] = stbi_load(argv[1], &width, &height, &bpp, 1);
			images[kl] = stbi_load(text, &width, &height, &bpp, 1);
			//uint8_t* image = stbi_load(argv[1], &width, &height, &bpp, 1);
			
			
			uint8_t* image = images[kl];
			//printf("Loaded image with height %d and width %d \n", width, height);
	uint8_t* edge_image;
    edge_image = malloc(width*height);

	int x,y;
        int val;
		double start_time = omp_get_wtime();
		//for(x= (thread_id*amount_work_x); x< (thread_id*amount_work_x)+amount_work_x; x++){
 		#pragma omp parallel for private(x,y,val) collapse(2)
		for(x= 0; x< width; x++){   	
		for(y= 0 ; y < height; y++){
			//getting number of threads
			if(nthreads==0) nthreads = omp_get_num_threads();
			//Applying gx sobel kernel
    		int pixel_x = ((sobel_x[0][0]*image[x*height-1+y-1])+ (sobel_x[0][1]* image[x*height+y-1]) + (sobel_x[0][2] * image[x*height+1+y-1]))+
    			      ((sobel_x[1][0]*image[x*height-1+y])+ (sobel_x[1][1]* image[x*height+y]) + (sobel_x[1][2] * image[x*height+1+y]))+
    			      ((sobel_x[2][0]*image[x*height-1+y+1])+ (sobel_x[2][1]* image[x*height+y+1]) + (sobel_x[2][2] * image[x*height+1+y+1]));
    		// Applying gy sobel kernel	      
    	    int pixel_y = ((sobel_y[0][0]*image[x*height-1+y-1])+ (sobel_y[0][1]* image[x*height+y-1]) + (sobel_y[0][2] * image[x*height+1+y-1]))+
    			      ((sobel_y[1][0]*image[x*height-1+y])+ (sobel_y[1][1]* image[x*height+y]) + (sobel_y[1][2] * image[x*height+1+y]))+
    			      ((sobel_y[2][0]*image[x*height-1+y+1])+ (sobel_y[2][1]* image[x*height+y+1]) + (sobel_y[2][2] * image[x*height+1+y+1]));
    		val = ceil(sqrt((pixel_x*pixel_x)+(pixel_y*pixel_y)));
    		edge_image[x*height+y] = val;	
    	}
    }
	//printf("\nNumber of threads : %d\n\n", nthreads);
	double time = omp_get_wtime() - start_time;
			// saving the processesd image frame
			stbi_image_free(image);    
			stbi_write_png(text, width, height, CHANNEL_NUM, edge_image, width*CHANNEL_NUM);
	
	}
	if (ProcessorRank == 0) //if root process
	{      
		
		printf("\nNumber of threads : %d\n\n", nthreads);
		//printf("Value of N : %d\n\n", n_iterations);
		stop_time = MPI_Wtime(); // can use this function to grab a
                                  // timestamp (in seconds)
		double exec_time = stop_time - start_time;
		printf("\nExecution time: %f\n", exec_time);
	}

    
    return 0;
}

