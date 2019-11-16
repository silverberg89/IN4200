#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[])
{
  double s = 0.0;
  int n = pow(10,8);
  double *a = malloc(n*sizeof(double));
  for (int i=0; i<n; i++) {

    a[i]=1;

  }
  clock_t t; 
  t = clock();
  for (int i=0; i<n; i++) {

    s += a[i]*a[i];

  }
  t = clock() - t; 
  double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds 
  printf("fun() took %f seconds to execute \n", time_taken); 
}