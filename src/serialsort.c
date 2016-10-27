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

int cmpfunc (const void * a, const void * b) {
  return ( *(char*)a - *(char*)b );
}

int main(int argc, char *argv[]) {

  char *dataSet;

  dataSet = malloc(sizeof(char)*DATA_SET_SIZE);

  int i;
  srand((unsigned) time(NULL));
  for(i = 0; i < DATA_SET_SIZE; i++) {
    dataSet[i] = rand() % 26 + 'a';
  }
  qsort(dataSet, DATA_SET_SIZE, sizeof(char), cmpfunc);
}
