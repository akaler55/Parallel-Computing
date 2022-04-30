#include <stdio.h>
#include <dirent.h>
  
int main(void)
{


/* 
creating frames of the input video.
*/

int imges_ln = -2;	
//######## rm -rf dir1
char cmd_1[] = "mkdir tmp3";
char cmd_2[] = "cp input_video.mp4 tmp3/input_video.mp4";
//char cmd_3[] = "cd tmp3";
//char cmd_4[] = "ffmpeg -i teapot.mp4 image%2d.png";
//char cmd_4[] = "ffmpeg -i tmp3/teapot.mp4 -vf fps=15 tmp3/image%d.bmp";
//char cmd_4[] = "ffmpeg -i tmp3/teapot.mp4 tmp3/image%d.bmp";
char cmd_4[] = "ffmpeg -i tmp3/input_video.mp4 -b:v 200000 tmp3/image%d.bmp";
char cmd_5[] = "rm tmp3/input_video.mp4";
char cmd_6[] = "rm -rf tmp3";

system(cmd_6);
system(cmd_1);
system(cmd_2);
//system(cmd_3);
system(cmd_4);
system(cmd_5);


DIR *dir;
struct dirent *ent;
if ((dir = opendir ("tmp3")) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    //printf ("%s\n", ent->d_name);
	imges_ln+=1;
  }
  
  printf("\n Total image Frames : %d \n", imges_ln);
  closedir (dir);
} else {
  /* could not open directory */
  perror ("");
  return 0;
}




 return 0;
}