#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
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
// ########################################################## Initialization
int iters;
float kappa;
char* input_jpeg_filename;
char* output_jpeg_filename;
struct timespec start, end;

// Activate for running in terminal----
// In terminal: make
// In terminal: ./serial_main 0.2 10 mona_lisa_noisy.jpg mona_lisa_worked.jpg
if (argc > 4)
{
    kappa = atof(argv[1]);
    iters = atoi(argv[2]);
    input_jpeg_filename = argv[3];
    output_jpeg_filename = argv[4];
}
else
{
  printf("Insert the following as your input:\n");
  printf("./serial_main *kappa* *iters* *input_file* *output_file*\n");
  return 1;
}

// ########################################################## Initilize
int x,y;                                // x = width, y = heigth
int num_components = 1;                 // 1 = Greyscale
unsigned char* image_chars;
image u, u_bar;                         // Create array holders as instructed 'image' type

// // Indecies of the corners
// image_chars[0] = 255;       // First element of first row
// image_chars[x-1] = 255;     // Last element of first row
// image_chars[x*(y-1)] = 255; // First element of last row
// image_chars[x*y-1] = 255;   // Last element of last row

// ########################################################## Run functions

import_JPEG_file(input_jpeg_filename, &image_chars, &y, &x, &num_components); // Import image

allocate_image(&u, x, y);                                                     // Allocate current 2d array

allocate_image(&u_bar, x, y);                                                 // Allocate future 2d array

convert_jpeg_to_image(image_chars, &u);                                       // Converts 1d data to 2d

clock_gettime(CLOCK_MONOTONIC, &start);
iso_diffusion_denoising(&u, &u_bar, kappa, iters);                            // Iterations of isotropic diffusion algorithm
clock_gettime(CLOCK_MONOTONIC, &end);

convert_image_to_jpeg(&u_bar, image_chars);                                   // Converts 2d data to 1d

deallocate_image (&u);                                                        // Free current 2d array

deallocate_image (&u_bar);                                                    // Free future 2d array

// ########################################################## Export image
int jpg_quality = 75;                                                        // Output quality [0,100]
export_JPEG_file (output_jpeg_filename, image_chars, y, x, num_components, jpg_quality);

double totalT = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1E9;
printf("\nFinished 'iso_diffusion_denoising' function in %f seconds\n",totalT);

return 0;
}