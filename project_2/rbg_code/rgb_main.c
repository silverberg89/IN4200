#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "mpi.h"
#include "functions.c"

#ifdef __MACH__
#include <stdlib.h>
#else 
#include <malloc.h>
#endif

void import_JPEG_file (const char* filename, unsigned char** image_chars,
                       int* image_height, int* image_width,
                       int* num_components);
void export_JPEG_file (const char* filename, const unsigned char* image_chars,
                       int image_height, int image_width,
                       int num_components, int quality);

int main(int argc, char *argv[])
{
// #########################   General Initialization
// // For running in terminal:
// // & make
// // & mpirun -n 3 ./rgb_main 0.1 10 mona_lisa_noisy.jpg mona_lisa_worked.jpg

int my_x,my_y,my_s,my_r,my_rank,num_procs,iters,x,y;
int message = 99;
int num_components = 3;
float kappa;
unsigned char* image_chars;
char* input_jpeg_filename,* output_jpeg_filename;
MPI_Status status;
image u, u_bar, whole_image;

if (argc > 4)
{
  kappa                   = atof(argv[1]);
  iters                   = atoi(argv[2]);
  input_jpeg_filename     = argv[3];
  output_jpeg_filename    = argv[4];
}
else
{
  printf("Insert the following as your input:\n");
  printf("mpirun -n *nr_of_processes* ./parallel_main *kappa* *iters* *input_file* *output_file*\n");
  return 1;
}

MPI_Init (&argc, &argv);                        // Deliver input arguments
MPI_Comm_rank (MPI_COMM_WORLD, &my_rank);       // Draw rank of process (Worker)
MPI_Comm_size (MPI_COMM_WORLD, &num_procs);     // Draw number of processes

if (num_procs < 2)
{
  printf("Must use at least two processes\n");
  MPI_Finalize();
  return 1;
}
// #########################   Import image process (Only need one worker)
if (my_rank==0)
{ 
  import_JPEG_file (input_jpeg_filename, &image_chars, &y, &x, &num_components); // Import image
  allocate_image3d (&whole_image, x, y, num_components);                         // Allocate current 3D array
  convert_jpeg_to_image (image_chars, & whole_image, num_components);
}
// #########################   Workers initilization
MPI_Bcast (&x, 1, MPI_INT, 0, MPI_COMM_WORLD);             // Data (x,y) communicated to everyone
MPI_Bcast (&y, 1, MPI_INT, 0, MPI_COMM_WORLD);

int workers = num_procs-1;                                 // Nr of workers
int section = x/workers;                                   // Partion size of image
int rest    = x-(section*workers);                         // Rest size of image
int stride  = 1;                                           // Used for 'see' neightbour

partitioning(section, &my_x, &my_s, &my_r, my_rank, workers, rest, stride); // Partion information for each worker
my_y = y;

allocate_image3d (&u, my_x, my_y, num_components);         // Allocate 3D array for smoothed image, for each worker
allocate_image3d (&u_bar, my_x, my_y, num_components);
// #########################   Smoothing process
MPI_Barrier (MPI_COMM_WORLD);                    // Wait for all workers for timing
double time_s = MPI_Wtime();   
int door_r = 1;                         // Doors for determine if a team should send or receive data
int door_s = 1;
for (int steps=0;steps<iters;steps++)   // Iterates over choosen steps
{
  if(my_rank == 0)                               // MASTER TEAM
  {
    for (int color=0;color<num_components;color++)                // Loop over components
    {
      for(int worker=1;worker<num_procs;worker++)                 // Loop over all workers
      {
        partitioning(section, &my_x, &my_s, &my_r, worker, workers, rest, stride); // Partion information for each worker
        if (door_s == 1)
        {   
          for(int i=0;i<my_y;i++)                                 // Master send data to workers
          {
            MPI_Send (& whole_image.image_data[color][i][my_s],my_x,MPI_FLOAT,worker,message,MPI_COMM_WORLD);
          }
        }
        else                                                      // Master recive data from workers
        {
          for(int i=0;i<my_y;i++)
          {
            MPI_Recv (& whole_image.image_data[color][i][my_s+stride],my_r,MPI_FLOAT,worker,message,MPI_COMM_WORLD,&status);
          }
        }
      }
    }
    door_s *= (-1);
  }
  else                                           // WORKER TEAM
  {
    for (int color=0;color<num_components;color++)                // Loop over components
    {
      if (door_r == 1)                   
      {
        for (int i=0;i<my_y;i++)                                  // Workers recive data from master
        {
          MPI_Recv (&u.image_data[color][i][0],my_x,MPI_FLOAT,0,message,MPI_COMM_WORLD,&status);
        }
        iso_diffusion_denoising_parallel(&u,&u_bar,kappa,my_x,stride,color);     // Smoothing algorithm called
      }
      else                                                        // Workers send data to master
      {
        for (int i=0;i<my_y;i++)
        {
          MPI_Send (&u.image_data[color][i][0],my_r,MPI_FLOAT,0,message,MPI_COMM_WORLD);
        }
      }
    }
    door_r *= (-1);
  }
}
double time_e = MPI_Wtime();  // Draws process with min and max time execution as well as the average of all processes.
double time_T = time_e-time_s;
double min_T,max_T,avg_T;
MPI_Reduce(&time_T,&min_T,1,MPI_DOUBLE,MPI_MIN,0,MPI_COMM_WORLD);
MPI_Reduce(&time_T,&max_T,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
MPI_Reduce(&time_T,&avg_T,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);

// #########################    Export process
if (my_rank == 0)               // Only use one worker
{
  int quality = 100;
  avg_T /= num_procs;
  printf("Minimum: %lf Avgerage: %lf Maximum: %lf\n",min_T,avg_T,max_T);
  convert_image_to_jpeg (&whole_image, image_chars, num_components);
  export_JPEG_file (output_jpeg_filename,image_chars,y,x,num_components,quality);
  free(image_chars);
  deallocate_image (&whole_image, num_components);
}
deallocate_image (&u, num_components);
deallocate_image (&u_bar, num_components);
MPI_Finalize();                 // Terminates MPI environment
return 0;
}