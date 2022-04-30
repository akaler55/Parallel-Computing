#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define CHANNEL_NUM 1

int main(int argc, char *argv[] ) {
    
    if(argc < 4){
    	printf("Usage : <img_in_name> <img_out_name> <number_of_threads>");
    	return -1;
    }
 
    int width, height, bpp;
	int nthreads = 0;
	// Sobel filter kernels
    int sobel_x[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    //int sobel_y[3][3] = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
	int sobel_y[3][3] = { {1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    int number_threads = atoi(argv[3]);
    //int thread_id[number_threads];

    uint8_t* image = stbi_load(argv[1], &width, &height, &bpp, 1);
    printf("Loaded image with height %d and width %d \n", width, height);

    uint8_t* edge_image;
    edge_image = malloc(width*height);

	int x,y;
    //#pragma omp parallel num_threads(number_threads) private(x,y)
    
        //int thread_id = omp_get_thread_num();
        
        int amount_work_y = (int)((height)/number_threads);
        int amount_work_x = (int)((width)/number_threads);
		
        int val;
		double start_time = omp_get_wtime();
		//for(x= (thread_id*amount_work_x); x< (thread_id*amount_work_x)+amount_work_x; x++){
 		#pragma omp parallel for private(x,y,val) collapse(2)
		for(x= 0; x< width; x++){   	
		for(y= 1 ; y < height; y++){
			if(nthreads==0) nthreads = omp_get_num_threads();
    		int pixel_x = ((sobel_x[0][0]*image[x*height-1+y-1])+ (sobel_x[0][1]* image[x*height+y-1]) + (sobel_x[0][2] * image[x*height+1+y-1]))+
    			      ((sobel_x[1][0]*image[x*height-1+y])+ (sobel_x[1][1]* image[x*height+y]) + (sobel_x[1][2] * image[x*height+1+y]))+
    			      ((sobel_x[2][0]*image[x*height-1+y+1])+ (sobel_x[2][1]* image[x*height+y+1]) + (sobel_x[2][2] * image[x*height+1+y+1]));
    			      
    	    int pixel_y = ((sobel_y[0][0]*image[x*height-1+y-1])+ (sobel_y[0][1]* image[x*height+y-1]) + (sobel_y[0][2] * image[x*height+1+y-1]))+
    			      ((sobel_y[1][0]*image[x*height-1+y])+ (sobel_y[1][1]* image[x*height+y]) + (sobel_y[1][2] * image[x*height+1+y]))+
    			      ((sobel_y[2][0]*image[x*height-1+y+1])+ (sobel_y[2][1]* image[x*height+y+1]) + (sobel_y[2][2] * image[x*height+1+y+1]));
    		val = ceil(sqrt((pixel_x*pixel_x)+(pixel_y*pixel_y)));
    		edge_image[x*height+y] = val;	
    	}
    }
	printf("\nNumber of threads : %d\n\n", nthreads);
	double time = omp_get_wtime() - start_time;
	printf("\nTotal Execution time (in sec):%f\n", time);
    

    // for(int i = 0; i<number_threads; i++){
    //     thread_id[i] = i;
    // 	struct threadInfo info = {
    //         image,
    // 		edge_image,
    //         width,
    // 		height,
    // 		number_threads,
    // 		thread_id[i]
    // 	};   
    // 	pthread_create(&threads[i], NULL, (void *) calculateFilter, &info);
    // }
    
    // for(int i = 0; i<number_threads; i++){
    // 	pthread_join(threads[i], NULL);
    // }
     
    // for(int x=1; x< width-2; x++){
    	
    // 	for(int y=1; y < height-2; y++){
    	
    // 		int pixel_x = ((sobel_x[0][0]*rgb_image[x*height-1+y-1])+ (sobel_x[0][1]* rgb_image[x*height+y-1]) + (sobel_x[0][2] * rgb_image[x*height+1+y-1]))+
    // 			      ((sobel_x[1][0]*rgb_image[x*height-1+y])+ (sobel_x[1][1]* rgb_image[x*height+y]) + (sobel_x[1][2] * rgb_image[x*height+1+y]))+
    // 			      ((sobel_x[2][0]*rgb_image[x*height-1+y+1])+ (sobel_x[2][1]* rgb_image[x*height+y+1]) + (sobel_x[2][2] * rgb_image[x*height+1+y+1]));
    			      
    // 	        int pixel_y = ((sobel_y[0][0]*rgb_image[x*height-1+y-1])+ (sobel_y[0][1]* rgb_image[x*height+y-1]) + (sobel_y[0][2] * rgb_image[x*height+1+y-1]))+
    // 			      ((sobel_y[1][0]*rgb_image[x*height-1+y])+ (sobel_y[1][1]* rgb_image[x*height+y]) + (sobel_y[1][2] * rgb_image[x*height+1+y]))+
    // 			      ((sobel_y[2][0]*rgb_image[x*height-1+y+1])+ (sobel_y[2][1]* rgb_image[x*height+y+1]) + (sobel_y[2][2] * rgb_image[x*height+1+y+1]));
    			      
    // 		int val = ceil(sqrt((pixel_x*pixel_x)+(pixel_y*pixel_y)));
    // 		edge_image[x*height+y] = val;	
  
    // 	}
    
    // }
    printf("Sobel edge filter was sucessfully applied to %s \n", argv[1]);
    
    stbi_image_free(image);    
    stbi_write_png(argv[2], width, height, CHANNEL_NUM, edge_image, width*CHANNEL_NUM);
    return 0;
}

