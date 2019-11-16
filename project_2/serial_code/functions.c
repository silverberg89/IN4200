#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

typedef struct      // Define struct for denoising possibilities != [0,255]
{
float** image_data; // 2D array of floats
int x;              // # Pixels in horizontal-direction
int y;              // # Pixels in vertical-direction
}
image;

void allocate_image(image *u, int x, int y)
/*
Allocate a 2D array.

Input args:
    u:              Pointer to current image struct holder
    x:              Horizontal dimention (Total width)
    y:              Vertical dimention (Total heigth)
*/
{
    (*u).y          = y;          // Gets '.y' from struct that 'u' points to
    (*u).x          = x;
    (*u).image_data = (float**) malloc(y*sizeof(float*));
    for (int i=0;i<y;i++)
    {
        (*u).image_data[i] = (float*) malloc(x*sizeof(float*));
    }
}

void convert_jpeg_to_image(const unsigned char* image_chars, image *u)
/*
Convert a 1D array of unsigned char values into an image struct (2D).

Input args:
    image_chars:    Imported image in 1d format, top-left corner = [0], top right corner = [x-1]
    u:              Pointer to current image struct holder
*/
{
    int index = 0;
    for (int i=0;i<(*u).y;i++)                                  // Loop over rows (heigth)
    {
        for (int j=0;j<(*u).x;j++)                              // Loop over cols (width)
        {
            index = ((*u).x)*i+j;                               // Jump a row by each (i)
            (*u).image_data[i][j] = (float)image_chars[index];  // Insert values at correct index
        }
    }
}

void iso_diffusion_denoising(image *u, image *u_bar, float kappa, int iters)
/*
Perform iterations of the isotropic diffusion algorithm.
It operates in the domain [1<i<x-2],[1<j<y-2].
It do not change the boundary elements of the image.

Input args:
    u:              Pointer to current image struct holder
    u_bar:          Pointer to future image struct holder
    kappa:          Physical constant (Usually 0.2 or smaller)
    iters:          Number of iterations to perform
*/

// Visual formation of 2d image:
// *   b   *
// a   c   e
// *   d   *
{
    float a,b,c,d,e;
    for (int loops=0;loops<iters;loops++)    // Loop over nr of iterations
    {
        for (int j=0;j<(*u).x;j++)             // Set boundaries top & bottom
        {
            (*u_bar).image_data[0][j]        = (*u).image_data[0][j]; 
            (*u_bar).image_data[(*u).y-1][j] = (*u).image_data[(*u).y-1][j]; 
        }
        for (int j=0;j<(*u).y;j++)           // Set boundaries sides
        {
            (*u_bar).image_data[j][0]        = (*u).image_data[j][0]; 
            (*u_bar).image_data[j][(*u).x-1]     = (*u).image_data[j][(*u).x-1]; 
        } 

        for (int i=1;i<(*u).y-1;i++)         // Loop over rows (heigth)
        {
            for (int j=1;j<(*u).x-1;j++)     // Loop over cols (width)
            {
                a = (*u).image_data[i][j-1]; // Isotropic diffusion algorithm
                b = (*u).image_data[i-1][j];
                c = (*u).image_data[i][j];
                d = (*u).image_data[i+1][j];
                e = (*u).image_data[i][j+1];

                (*u_bar).image_data[i][j] = c + kappa*(b+a-4*c+e+d);
            } 
        }

        for (int i=0;i<(*u).y;i++)           // Loop for updating smoother image
        {
            for (int j=0;j<(*u).x;j++)
            {
                (*u).image_data[i][j] = (*u_bar).image_data[i][j];
            } 
        }
    }
}

void convert_image_to_jpeg(const image *u, unsigned char* image_chars)
/*
Convert a 2D array of an image struct to a unsigned char values array (1D).

Input args:
    image_chars:    Imported image in 2d format, top-left corner = [0][0], top right corner = [0][x-1]
    u:              Pointer to current image struct holder
*/
{
    int index = 0;
    for (int i=0;i<(*u).y;i++)                                  // Loop over rows (heigth)
    {
        for (int j=0;j<(*u).x;j++)                              // Loop over cols (width)
        {
            index = ((*u).x)*i+j;                               // Jump a row each (i)
            image_chars[index] = (*u).image_data[i][j];         // Insert values at correct index
        }
    }
}

void deallocate_image(image *u)
/*
Deallocates a 2D array.

Input args:
    u:              Pointer to current image struct holder
*/
{
    // 'Avoid memory leaks by: for each malloc(), there is one corresponding free()
    for (int i=0;i<(*u).y;i++)
    {
        free((*u).image_data[i]);
    }
    free((*u).image_data);
}