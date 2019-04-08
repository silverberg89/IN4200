#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>
#include "PE_functions_15319.c"

//int main()
int main(int argc, char *argv[])
{
  // Set up time for clocking the task
  double start_omp = omp_get_wtime();
  // -----------------------------------------------------------------------
  // [1] Start reading the data
  double start_read_omp = omp_get_wtime();
  printf("# Start data extraction: \n");

  // // Input arguments
  // // In terminal: gcc PE_main_15319.c -o PE_main_15319 -lm -fopenmp
  // // In terminal: ./PE_main_15319 web-NotreDame.txt 1.0e-5 0.85 10
  // char *filename = argv[1];
  // double epsilon = strtod(argv[2], NULL);
  // double d       = strtod(argv[3], NULL);
  // int M          = atoi(argv[4]);

  //char* filename     = "web-NotreDame.txt";
  //char* filename     = "100nodes_graph.txt";
  char* filename     = "webgraph.txt";
  double d           = 0.85;
  double epsilon     = 1.0e-5;
  int M              = 100;

  int print_arrays   = 0;                                 // Set to 1 if you want to inspect the arrays [row_idx,col_idx,val,L_vec]

  // First data extraction / construction
  int Nodes       = 0;
  int Edges       = 0;
  int Dangs       = 0;

  read_graph_from_file_0(filename,&Nodes,&Edges);
  if (M>Nodes)
  {
    printf("To many top pages choosen, max would be: %d\n",Nodes);
    return 1;
  }

  // Second data extraction / construction
  int *L_vec      = calloc(Nodes, sizeof *L_vec);          // Count nr of outgoing connections [NOTE: If L_vec[i] = 0 : Dangler page]
  int *row_idx    = calloc(Edges, sizeof *row_idx);        // Keep track of row indices
  int *col_idx    = calloc(Edges, sizeof *col_idx);        // Keep track of col indices
  read_graph_from_file_1(filename,&Dangs,&Nodes,&Edges,col_idx,row_idx,L_vec);

  // Third data extraction / construction
  double *val     = calloc(Edges, sizeof *val);            // CRS unsorted matrix in 1D form
  read_graph_from_file_2(&Dangs,&Nodes,&Edges,L_vec,col_idx,val);

  double end_read_omp = omp_get_wtime();
  double tot_read_omp = end_read_omp-start_read_omp;
  printf("Completed data extraction in: %f seconds [omp time]\n", tot_read_omp);
  // -----------------------------------------------------------------------

  // Print arrays for verification
  if (print_arrays == 1)
  {
    Array_test(&Nodes,&Edges,L_vec,col_idx,row_idx,val);
  }

  // -----------------------------------------------------------------------
  // [2] Start PageRank calculation
  printf("\r# Start PageRank algorithm:\n");
  double start_page_omp = omp_get_wtime();

  int *index      = calloc(Nodes, sizeof *index);          // Allocate the index array for all pages
  double *X_1     = calloc(Nodes, sizeof *X_1);            // Allocate the future score vector  "X[k]"
  PageRank_iterations(&Dangs,&Nodes,&Edges,&epsilon,&d,index,X_1,L_vec,col_idx,row_idx,val);
  
  double end_page_omp = omp_get_wtime();
  double tot_page_omp = end_page_omp-start_page_omp;
  printf("Completed PageRank algorithm in: %f seconds [omp time]\n", tot_page_omp);

  free(val);                                               // Free memory allocation
  free(L_vec);
  free(col_idx);
  free(row_idx);
  // -----------------------------------------------------------------------
  // [3] Start searching for the top webpages
  printf("\r# Start search for top %d webpages: \n",M);
  double start_sort_omp   = omp_get_wtime();
  top_n_webpages(&M,&Nodes,X_1,index);

  double end_sort_omp = omp_get_wtime();
  double tot_sort_omp = end_sort_omp-start_sort_omp;
  printf("Sorting top %d webpages took: %f seconds [omp time]\n", M, tot_sort_omp);

  // Free memory allocation
  free(X_1);
  free(index);

  double end_omp = omp_get_wtime();
  double tot_omp = end_omp-start_omp;
  printf("\rCompleted task in: %f seconds [omp time]\n", tot_omp);

  return 0;
}