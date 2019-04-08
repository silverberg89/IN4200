#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <omp.h>

void read_graph_from_file_0(char *filename, int *Nodes, int *Edges)
/// -----------------------------------------------------------
// # Read file and extract data
// Input:
// * filename:  Holding the name of the web graph
// * Nodes:     Empty parameter
// * Edges:     Empty parameter
// Changed output:
// * Edges:     Nr of Edges
// * Nodes:     Nr of Nodes
/// -----------------------------------------------------------
{
    FILE *infile;
    infile = fopen(filename, "r");
    fscanf(infile, "%*[^\n]\n");                               // Skip lines to relevant data
    fscanf(infile, "%*[^\n]\n");
    fscanf(infile, "# Nodes: %d Edges: %d\n",&*Nodes,&*Edges); // Extract data
}
void read_graph_from_file_1(char *filename,int *Dangs, int *Nodes, int *Edges, int *col_idx, int *row_idx, int *L_vec)
/// -----------------------------------------------------------
// # Read file and extract data
// Input:
// * filename:  Holding the name of the web graph
// * Dangs:     Empty parameter
// * Nodes:     Empty parameter
// * Edges:     Empty parameter
// * col_idx:   Zero array of size Nodes
// * row_idx:   Zero array of size Nodes
// * L_vec:     Zero array of size Nodes
// Changed output:
// * Dangs:     Nr of dangling webpages
// * Edges:     Nr of Edges after self links are removed
// * col_idx:   Array holding column indices of the outgoing connections
// * row_idx:   Array holding row indices of the outgoing connections
// * L_vec:     Array holding nr of outgoing connections in one column
/// -----------------------------------------------------------
{
    int y           = 0;
    int x           = 0;
    int idx_ext     = 0;

    FILE *infile;
    infile = fopen(filename, "r");
    fscanf(infile, "%*[^\n]\n");                 // Skip lines to relevant data
    fscanf(infile, "%*[^\n]\n");
    fscanf(infile, "%*[^\n]\n");
    fscanf(infile, "%*[^\n]\n");
    while (fscanf(infile, "%i %i",&x,&y) != EOF) // Scan to end of file
    {
        if (x == y)
        {
            continue;                            // Assure we skip self links
        }
        col_idx[idx_ext]        = x;             // Saves row_idx
        row_idx[idx_ext]        = y;             // Saves col_idx
        L_vec[x]                +=1;             // Collect outgoing connections
        idx_ext                 +=1;
    }
    fclose (infile);

    for (int i = 0;i<*Nodes;i++)                 // Calculate nr of dangling webpages
    {
        if (L_vec[i] == 0)
        {
            *Dangs +=1;
        }
    }

    printf("Nr of dangling webpages: %d\n",*Dangs); 
    *Edges = *Edges-(*Edges-idx_ext);            // Edges without self links
    printf("Found %d Nodes and %d Edges after removal of self links\n", *Nodes, *Edges);
}

void read_graph_from_file_2(int *Dangs, int *Nodes, int *Edges, int *L_vec, int *col_idx, double *val)
/// -----------------------------------------------------------
// # Extract data
// Input:
// * Dangs:     Nr of dangling webpages
// * Nodes:     Nr of Nodes
// * Edges:     Nr of Edges after self links are removed
// * col_idx:   Array holding column indices of the outgoing connections
// * row_idx:   Array holding row indices of the outgoing connections
// * L_vec:     Array holding nr of outgoing connections in one column
// * val:       Zero array of size Edges
// Changed output:
// * val:       Array holding the data in a CRS 1D format
/// -----------------------------------------------------------
{
    int i;
    double temp = 0;
    double val_sum = 0;
    #pragma omp parallel for reduction(+:val_sum) num_threads(3)                            // OMP AREA (Do not increase efficency for more threads)
    for (i = 0;i<*Edges;i++)
    {
        val[i] = (double)1/L_vec[col_idx[i]];
        val_sum += val[i];                                                                  // Test: If (val_sum + danglings)/Nodes != 1.0 then something is wrong
    }

    printf("Test initial sum(val) [1.0?]: %f\n",(val_sum+(*Dangs))/(*Nodes));
}

void Array_test(int *Nodes,int *Edges,int *L_vec, int *col_idx, int *row_idx, double *val)
/// -----------------------------------------------------------
// # Prints the critical arrays
// Input:
// * Nodes:     Nr of Nodes
// * Edges:     Nr of Edges after self links are removed
// * col_idx:   Array holding column indices of the outgoing connections
// * row_idx:   Array holding row indices of the outgoing connections
// * L_vec:     Array holding nr of outgoing connections in one column
// * val:       Array holding the data in a CRS 1D format
// Changed output:
// * Prints of the element values of each array
/// -----------------------------------------------------------
{
    for (int i=0;i<*Edges;i++)
    {
        printf("row_idx[%d] %d\n",i,row_idx[i]);
    }
    printf("----\n");
    for (int i=0;i<*Nodes;i++)
    {
        printf("L[%d] %d\n",i,L_vec[i]);
    }
    printf("----\n");
    for(int i=0;i<*Edges;i++)
    {
        printf("col_idx_unsorted[%d] %d\n",i,col_idx[i]);
    }
    printf("----\n");
    for(int i=0;i<*Edges;i++)
    {
        printf("val[%d,%d]: %f\n",row_idx[i],col_idx[i],val[i]);
    }
}

void PageRank_iterations(int *Dangs, int *Nodes,int *Edges, double *epsilon, double *d, int *index, double *X_1, int *L_vec, int *col_idx, int *row_idx, double *val)
/// -----------------------------------------------------------
// Calculation of the PageRank algorithm
// Inputs:
// * Dangs:     Nr of dangling webpages
// * Nodes:     Nr of Nodes
// * Edges:     Nr of Edges after self links are removed
// * epsilon:   Input error threshold
// * d:         Input damping constant
// * index:     Zero array of size Nodes
// * X_1:       Zero array of size Nodes
// * val:       Holding nonzero values.
// * col_idx:   column index of each nonzero value.
// * row_ptr:   Contains indices of where a new row start.
// * L_vec:     Contains outgoing connections and dangler information
// Outputs:
// * X_1        Holding the final scores
// * index:     Holding the indices of the webpages
// * ../scores_unsorted.txt:    A list of the scores unsorted
/// -----------------------------------------------------------
{
    // Define items
    double *X       = calloc(*Nodes, sizeof *X);    // Allocate the current score vector "X[k-1]"
    double W        = 0.0;                          // Dangling scalar
    double guess    = 1/(double)(*Nodes);           // Initial guess
    double constant = (1.0-(*d))/(*Nodes);          // Pre-calculated constants for PageRank loop
    double constant_1 = (*d)/(*Nodes);
    double term_1   = 0.0;                          // Pre-calculated term
    double delta    = 10*(*epsilon);                // While loop start/stop factor
    double X_sum    = 0.0;                          // Used for testing such that the sum is close to 1
    double e_temp   = 0.0;                          // Pre-calculated error in loop
    int iterations  = 0;                            // Count nr of iterations in while loop
    int threads     = 4;                            // Choose how many threads are to be used by openMP
    int f;
    int k;
    int g;

    while (delta > (*epsilon))
    {
        delta       = 0;                   // Reset data
        X_sum       = 0;
        W           = 0;

        if (iterations != 0)
        {
            #pragma omp parallel for private(f) reduction(+:W) num_threads(threads)       // OMP area: 4 threads slows this down, 3 is optimal
            for (f=0;f<*Nodes;f++)
            {
                X[f] = X_1[f];          // Copies the last calculation: "X(k-1)"
                X_1[f] = 0;             // Reset: "X(k)
                if (L_vec[f] == 0)
                {
                    W += X[f];          // Calculates: "W(k-1)"
                }
            }
        }
        else
        {
            #pragma omp parallel for private(f) reduction(+:W) num_threads(threads)                      // OMP area: Improves with nr of threads
            for (f=0;f<*Nodes;f++)
            {
                X[f] = guess;           // Initilize "X(k-1)"
                if (L_vec[f] == 0)
                {
                    W += X[f];          // Calculates: "W(k-1)"
                }
            }
        }

        //#pragma omp parallel for reduction(+:X_1[row_idx[k]]) num_threads(1)          // OMP area: Not possible due to indirect mapping as consequence of unsorted storage
        for (k = 0; k<*Edges; k++)
            {
                X_1[row_idx[k]] += ((*d)*val[k]*X[col_idx[k]]);
            }

        term_1 = (double)(constant + constant_1*W);                                     // Outside loop => Save computational time.
        #pragma omp parallel for private(g) reduction(+:X_sum,delta) num_threads(threads)     // OMP area: 4 threads slows this down, 3 or 2 is optimal
        for (g = 0; g<*Nodes; g++)
        {
            X_1[g] += term_1;                                                           // Adds the dangling weight term to the scores
            delta += fabs(X_1[g]-X[g]);                                                 // Calculate the absolute error between two iterations of score values
            X_sum += X_1[g];                                                            // Calculate total sum of score vector as a test, should go to 1.0
        }

        iterations  +=1;
    }

    printf("Test last sum(X) [~1.0?]: %f\n",X_sum);
    printf("The PageRank scores converged in: %d itterations\n",iterations);

    // Write to file and initilize index array
    FILE *asciifile;
    printf("Unsorted scores can be found at ../scores_unsorted.txt\n");
    asciifile = fopen("scores_unsorted.txt","w");
    for (int i=0; i<*Nodes; i++)
    {
        index[i] = i;
        fprintf(asciifile, "X[%d]: %.10f\n", i, X_1[i]);
    }
    fclose(asciifile);

    free(X);
}

void top_n_webpages(int *M, int *Nodes, double *X_1, int *index)
/// -----------------------------------------------------------
// # Finds the top webpages
// Inputs:
// * M:                   Input nr of webpages in list
// * Nodes:               Nr of Nodes
// * X_1                  Holding the final scores
// * index:               Holding the indices of the webpages
// Changed outputs:
// * ../scores_top.txt:   A list of the top scores of size M
/// -----------------------------------------------------------

{
    double *X_top           = calloc(*M, sizeof *X_top);
    if (*M<8)
    {
        // If wanted list is small [M<8]: do not have to use OMP.
        double max              = 0.0;
        int count               = 0;
        for (int z=0; z<*M;z++)
        {
            max = -1.0;
            for(int j=0;j<*Nodes;j++)
            {
                if (X_1[j] > max)
                {
                    max = X_1[j];       // Identifies remaining maximum of list
                    count = j;          // Keep track of page index
                }
            }
            X_top[z] = max;
            index[z] = count;
            X_1[count] = -1.0;          // Eliminates the maximum value for next iteration
        }
    }
    else
    {
        // If wanted list is large [M>8]: OMP can be used to lower computational time
        int batch[3]            = {(*Nodes/4), (*Nodes/4)*2, (*Nodes/4)*3};             // Batches
        int *count              = calloc(4, sizeof *count);                             // Hold indices of max scores in each batch
        double *max             = calloc(4, sizeof *max);                               // Hold values of max scores in each batch
        double temp_m           = -0.5;
        int temp_c              = 0;
        int j;
        for (int z=0; z<*M;z++)
        {
            #pragma omp parallel private(j)        // OMP area: 2 threads = 120 sec, 4 threads = 66 sec (Need private(j))
            {
                #pragma omp sections
                {
                    max[0] = max[1] = max[2] = max[3] = -1.0;
                    #pragma omp section            // First thread
                    for(j=0;j<batch[0];j++)
                    {
                        if (X_1[j] > max[0])
                        {
                            max[0] = X_1[j];       // Identifies remaining maximum of list
                            count[0] = j;          // Keep track of page index
                        }
                    }
                    #pragma omp section            // Second thread
                    for(j=batch[0];j<batch[1];j++)
                    {
                        if (X_1[j] > max[1])
                        {
                            max[1] = X_1[j];
                            count[1] = j;
                        }
                    }
                    #pragma omp section            // Third thread
                    for(j=batch[1];j<batch[2];j++)
                    {
                        if (X_1[j] > max[2])
                        {
                            max[2] = X_1[j];
                            count[2] = j;
                        }
                    }
                    #pragma omp section            // Fourth thread
                    for(j=batch[2];j<*Nodes;j++)
                    {
                        if (X_1[j] > max[3])
                        {
                            max[3] = X_1[j];
                            count[3] = j;
                        }
                    }
                }
            }

            temp_m  = -0.5;
            temp_c  = 0;
            for(j=0;j<4;j++)                        // Compare largest value from each batch
            {
                if (max[j]>temp_m)
                {
                    temp_m = max[j];
                    temp_c = count[j];
                }
            }
            X_top[z] = temp_m;                      // Assign values
            index[z] = temp_c;
            X_1[temp_c] = -1.0;                     // Eliminates the largest value
        }
    }

    // Write to file
    FILE *asciifile;
    printf("Top scores can be found at ../scores_top.txt\n");
    asciifile = fopen("scores_top.txt","w");
    for (int i=0; i<*M; i++)
    {
        fprintf(asciifile, "X[%d]: %.10f\n", index[i], X_top[i]);
    }
    fclose(asciifile);

    free(X_top);

    // ######################################################
    // // Alternative "Odd/Even sorting", it is quite equal in time for very large M
    // // But it's very slow if M is small or medium in size.
    // ######################################################
    // printf("\rStart full sorting of top webpages: \n");
    // int i;
    // int tempo1;
    // double tempo;
    // double start_sort_omp = omp_get_wtime();
    // for(int j=0;j<*Nodes;j++)
    // {
    //     if (j%2 == 0)   // Then Even
    //     {
    //         #pragma omp parallel for num_threads(4) shared(X_1,index,Nodes) private(i,tempo,tempo1)  // OMP area: Improves with nr of threads
    //         for(i=1;i<*Nodes;i+=2)
    //         {
    //             if (X_1[i-1] < X_1[i])
    //             {
    //                 tempo    = X_1[i];      // Swap procedure for score and index
    //                 X_1[i]   = X_1[i-1];
    //                 X_1[i-1] = tempo;
    //                 tempo1    = index[i];
    //                 index[i]   = index[i-1];
    //                 index[i-1] = tempo1;
    //             }
    //         }
    //     }
    //     else            // Then Odd
    //     {
    //         #pragma omp parallel for num_threads(4) shared(X_1,index,Nodes) private(i,tempo,tempo1)  // OMP area: Improves with nr of threads
    //         for(i=1;i<*Nodes-1;i+=2)
    //         {
    //             if (X_1[i] < X_1[i+1])
    //             {
    //                 tempo    = X_1[i];      // Swap procedure for score and index
    //                 X_1[i]   = X_1[i+1];
    //                 X_1[i+1] = tempo;
    //                 tempo1    = index[i];
    //                 index[i]   = index[i+1];
    //                 index[i+1] = tempo1;
    //             }
    //         }
    //     }
    // }
    // double end_sort_omp = omp_get_wtime();
    // double tot_sort_omp = end_sort_omp-start_sort_omp;
    // printf("Sorting webpages took: %f seconds [omp]\n", tot_sort_omp);
    // printf("Top scores can be found at ../scores_top.txt\n");

    // // Write to file
    // FILE *asciifile;
    // asciifile = fopen("scores_top.txt","w");
    // for (int i=0; i<*M; i++)
    // {
    //     fprintf(asciifile, "X[%d]: %.10f\n", index[i], X_1[i]);
    // }
    // fclose(asciifile);
}