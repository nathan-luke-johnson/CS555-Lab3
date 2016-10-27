/* Parsort
 * author: Luke Johnson
 * compile: mpicc -g parsort.c -o parsort
 * run: mpiexec -np ## ./parsort <size> <type> <yorn> where
 * size: Size of array of doubles to sort (on each processor)
 * type: m for merge sort, o for odd/even sort
 * yorn: y to display data, n to skip display
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>


int main(int argc, char *agrv[]) {
  
  MPI_Init(&argc, &argv);

  int myRank;
  int size;
  char *dataSet;
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  
  

  if(myRank == 0) {
    dataSet = malloc(sizeof(double)*size*
}

}
