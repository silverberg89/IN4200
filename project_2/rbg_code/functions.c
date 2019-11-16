#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

typedef struct       // Define struct
{
float*** image_data; // 3D array of floats
int x;               // # Pixels in horizontal-direction
int y;               // # Pixels in vertical-direction
}
image;

void allocate_image3d(image *u, int x, int y, int num_components)
/*
Allocate a 2D array.

Input args:
    u:              Pointer to current image struct holder
    x:              Horizontal dimention (Total width)
    y:              Vertical dimention (Total heigth)
    num_components: Number of components in image
*/
{
    (*u).y          = y;          // Gets '.y' from struct that 'u' points to
    (*u).x          = x;
    (*u).image_data = (float***) malloc(num_components*sizeof(float**));
    for (int g=0;g<num_components;g++)
    {
        (*u).image_data[g] = (float**) malloc(y*sizeof(float*));
        for (int i=0;i<y;i++)
        {
            (*u).image_data[g][i] = (float*) malloc(x*sizeof(float));
        }
    }
}

void convert_jpeg_to_image(const unsigned char* image_chars, image *u, int num_components)
/*
Convert a 1D array of unsigned char values into an image struct (3D).

Input args:
    image_chars:    Imported image in 1d format [rgbrgbrgb...]
    u:              Pointer to current image struct holder
    num_components: Number of components in image
*/
{
    int ix = 0;
    int iy = 0;
    for (int h=0;h<(*u).y*(*u).x*num_components;h+=3)           // Loop over all componentvalues
    {
        (*u).image_data[0][iy][ix] = (float)image_chars[h];     // Reads in (r,g,b) for a pixel
        (*u).image_data[1][iy][ix] = (float)image_chars[h+1];
        (*u).image_data[2][iy][ix] = (float)image_chars[h+2];
        ix +=1;
        if (ix==(*u).x)                                         // Controll over rows
        {
            ix = 0;
            iy +=1;
        }
    }
}

void iso_diffusion_denoising_parallel(image *u, image *u_bar, float kappa, int my_x, int stride, int color)
/*
Perform iterations of the isotropic diffusion algorithm.
It operates in the domain [1<i<my_x],[1<j<y-1].
It do not change the boundary elements of the image.

Input args:
    u:              Pointer to current image struct holder
    u_bar:          Pointer to future image struct holder
    kappa:          Physical constant (Usually 0.2 or smaller)
    iters:          Number of iterations to perform
    my_x:           Number of columns for worker
    stride:         How many columns the image shall "see" over boundaries
    color:          Which component should be worked on (r,g,b) = (0,1,2)
*/

// Visual formation of 2d image:
// *   b   *
// a   c   e
// *   d   *
{
    for (int j=0;j<my_x;j++)             // Set boundaries top & bottom
    {
        (*u_bar).image_data[color][0][j]        = (*u).image_data[color][0][j]; 
        (*u_bar).image_data[color][(*u).y-1][j] = (*u).image_data[color][(*u).y-1][j]; 
    }
    for (int j=0;j<(*u).y;j++)           // Set boundaries sides
    {
        (*u_bar).image_data[color][j][0]        = (*u).image_data[color][j][0]; 
        (*u_bar).image_data[color][j][my_x]     = (*u).image_data[color][j][my_x]; 
    } 

    float a,b,c,d,e;
    for (int i=1;i<(*u).y-1;i++)         // Loop over rows
    {
        for (int j=1;j<my_x;j++)         // Loop over cols
        {
            a = (*u).image_data[color][i][j-1]; // Isotropic diffusion algorithm
            b = (*u).image_data[color][i-1][j];
            c = (*u).image_data[color][i][j];
            d = (*u).image_data[color][i+1][j];
            e = (*u).image_data[color][i][j+1];

            (*u_bar).image_data[color][i][j] = c + kappa*(b+a-4*c+e+d);
        } 
    }
    
    for (int i=0;i<(*u).y;i++)           // Loop for updating smoother image
    {
        for (int j=1;j<my_x;j++)
        {
            (*u).image_data[color][i][j-stride] = (*u_bar).image_data[color][i][j]; 
        } 
    }
}

void convert_image_to_jpeg(const image *u, unsigned char* image_chars, int num_components)
/*
Convert a 3D array of an image struct to a unsigned char values array (1D).

Input args:
    image_chars:    Imported image in 3d format
    u:              Pointer to current image struct holder
    num_components: Number of components in image
*/
{
    int ix = 0;
    int iy = 0;
    for (int h=0;h<(*u).y*(*u).x*num_components;h+=3)   // Loop over all componentvalues
    {
        image_chars[h] = (*u).image_data[0][iy][ix];    // Reads in (r,g,b) for a pixel
        image_chars[h+1] = (*u).image_data[1][iy][ix];
        image_chars[h+2] = (*u).image_data[2][iy][ix];
        ix +=1;
        if (ix==(*u).x)                                 // Controll over rows
        {
            ix = 0;
            iy +=1;
        }
    }
}

void deallocate_image(image *u, int num_components)
/*
Deallocates a 3D array.

Input args:
    u:              Pointer to current image struct holder
    num_components: Number of components in image
*/
{
    for (int g=0;g<num_components;g++)
    {
        for (int i=0;i<(*u).y;i++)
        {
            free((*u).image_data[g][i]);
        }
        free((*u).image_data[g]);
    }
    free((*u).image_data);
}

void partitioning(int section, int* my_x, int* my_s, int* my_r, int my_rank, int workers, int rest, int stride)
/*
Partitioning data boundaries for workers.

Input args:
    section:              Equal sized partion of the x-dim in image
    my_x:                 Number of columns a worker is assigned to recieve from master
    my_s:                 Index where the first column is drawn 
    my_r:                 Number of columns a worker is assigned to deliver to master
    my_rank:              Worker ID
    workers:              Number of workers
    rest:                 Partion of image in x-dim that was left out of partitioning.
*/
{
    if (my_rank == workers)     // Last worker
    {
        *my_x = section+stride+rest;
        *my_r = section+rest;
        *my_s = (my_rank-1)*section-stride;
    }
    else if (my_rank == 1)      // First worker 
    {
        *my_x = section+stride;
        *my_r = section;
        *my_s = 0;
    }
    else                         // Middel workers
    {
        *my_x = section+stride*2;
        *my_r = section;
        *my_s = (my_rank-1)*section-stride;
    }
}