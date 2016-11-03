/* Serial Sort Program
 * This program is intended to be a basis for the MPI parallel sort.
 * It also serves as a serial baseline for comparison
 * to compile: gcc -g serialsort.c -o serialsort
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_SET_SIZE 1048576
//#define DATA_SET_SIZE 1024

int cmpfunc(const void * a, const void * b) {
  if (*(double*)a > *(double*)b) return 1;
  else if (*(double*)a < *(double*)b) return -1;
  return 0;
}

int main(int argc, char *argv[]) {

  double *dataSet;
  int dataSetSize = 0;
  if(argc>1) {
    dataSetSize = atoi(argv[1]);
  } else { dataSetSize = DATA_SET_SIZE; }

  dataSet = malloc(sizeof(double)*dataSetSize);

  int i;
  srand((unsigned) time(NULL));
  for(i = 0; i < dataSetSize; i++) {
    dataSet[i] = rand();
  }
  clock_t startTime;
  clock_t endTime;
  startTime = clock();
  qsort(dataSet, dataSetSize, sizeof(double), cmpfunc);
  endTime = clock();
  printf("sort took %f seconds.\n", (double)(endTime-startTime)/(double)CLOCKS_PER_SEC);
}
