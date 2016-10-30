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
#include <string.h>

#define MAX_SIZE 1048576

int argError(MPI_Comm comm);
int cmpfunc (const void * a, const void * b);
void sortedMerge(double *left, double *right, int size);
void mergeLow(double *a, double *b, double *c, int n);
void mergeHigh(double *a, double *b, double *c, int n);

int main(int argc, char *argv[]) {

  MPI_Init(&argc, &argv);

  int myRank;
  int P;
  int size;
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
  MPI_Comm_size(MPI_COMM_WORLD, &P);
  
  //printf("argc: %d\n", argc);

  char type;
  char yorn;
  double *dataSet;
  double *localBuffer;  
  int i = 1;
  if(argc < 4) { return argError(MPI_COMM_WORLD); }
  
  size = atoi(argv[i++]);
  //printf("size: %d\n", size);
  type = argv[i++][0];
  //printf("type: %c\n",type);
  yorn = argv[i++][0];
  //printf("yorn: %c\n", yorn);

  if(size < 1 || size > MAX_SIZE) { return argError(MPI_COMM_WORLD); }
  if(type != 'm' && type != 'o') { return argError(MPI_COMM_WORLD); }
  if(yorn != 'y' && yorn != 'n') { return argError(MPI_COMM_WORLD); }
  

  //Seed the random number generator
  //srand((unsigned) time(NULL));
  //Fill the data set
  dataSet = malloc(sizeof(double)*P*size);
  if(myRank == 0) {
    //printf("original:\n");
    for(i = 0; i < P*size; i++) {
      dataSet[i] = ((double) rand()) / RAND_MAX;
      //printf("%f\n", dataSet[i]); //print original for debugging
    }
  }
  localBuffer = malloc(sizeof(double)*size*P);
  double *thirdBuffer = malloc(sizeof(double)*size*P);
  // Scatter the data
  MPI_Scatter(
    dataSet, size, MPI_DOUBLE,
    localBuffer, size, MPI_DOUBLE,
    0, MPI_COMM_WORLD);
  //printf("Processor %d got datas!\n", myRank);
  //do local sort
  qsort(localBuffer, size, sizeof(double), cmpfunc);

  //copy contents of buffer to 'dataset'
  memcpy(dataSet, localBuffer, sizeof(double)*size);
  
  double startTime;
  double endTime;
  startTime = MPI_Wtime();
  
  //Now we do the complicated part
  //What this is will vary based on sort type.
  if(P > 1){
  if(type == 'm') { //do merge sort
    int gap;
    for(gap = 1; gap < P; gap *= 2) {
      //MPI_Barrier(MPI_COMM_WORLD);
      if ((myRank/gap)%2 != 0) { 
        //send data to p-gap
        //printf("gap: %d Proc %d sending to %d\n", gap, myRank, myRank-gap);
        MPI_Send(dataSet, size*gap,  MPI_DOUBLE,
                 myRank-gap, 0, MPI_COMM_WORLD);
        break;
      } else {
        //printf("gap: %d Proc %d receiving from %d\n", gap, myRank, myRank+gap);
        MPI_Recv(localBuffer, size*gap, MPI_DOUBLE,
                 myRank+gap, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //now do mergeSort with dataSet and localBuffer, storing into dataSet
        sortedMerge(dataSet, localBuffer, size*gap);
      }
    }
  } else { //do odd/even sort
    for(i = 0; i < P; i++) {
      memcpy(thirdBuffer, localBuffer, sizeof(double)*size);
      if(myRank%2 != 0) {
        if (i%2 == 0) {
          //printf("proc %d sending to %d", myRank, myRank-1);
          MPI_Send(localBuffer, size, MPI_DOUBLE,
	           myRank-1, 0, MPI_COMM_WORLD);
	  MPI_Recv(dataSet, size, MPI_DOUBLE,
	           myRank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  mergeHigh(localBuffer, dataSet, thirdBuffer, size);
	  memcpy(localBuffer, thirdBuffer, sizeof(double)*size); 
        } else if(myRank <= P-2) {
	  //printf("proc %d sending to %d", myRank, myRank+1);
	  MPI_Send(localBuffer, size, MPI_DOUBLE,
	           myRank+1, 0, MPI_COMM_WORLD);
	  MPI_Recv(dataSet, size, MPI_DOUBLE,
	           myRank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  mergeLow(localBuffer, dataSet, thirdBuffer, size);
	  memcpy(localBuffer, thirdBuffer, sizeof(double)*size);
         }
      } else {
        if (i%2 ==0) {
	  //printf("proc %d sending to %d", myRank, myRank+1);
	  MPI_Recv(dataSet, size, MPI_DOUBLE,
	           myRank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  MPI_Send(localBuffer, size, MPI_DOUBLE,
	           myRank+1, 0, MPI_COMM_WORLD);
          mergeLow(localBuffer, dataSet, thirdBuffer, size);
	  memcpy(localBuffer, thirdBuffer, sizeof(double)*size);
	} else if(myRank >= 1) { 
	  //printf("proc %d sending to %d", myRank, myRank-1);
	  MPI_Recv(dataSet, size, MPI_DOUBLE,
	           myRank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  MPI_Send(localBuffer, size, MPI_DOUBLE,
	           myRank-1, 0, MPI_COMM_WORLD);
	  mergeHigh(localBuffer, dataSet, thirdBuffer, size);
	  memcpy(localBuffer, thirdBuffer, sizeof(double)*size);
        }
      }
    }
    //All together now.
    MPI_Gather(localBuffer, size, MPI_DOUBLE,
               dataSet, size, MPI_DOUBLE,
	       0, MPI_COMM_WORLD);
  } }

  endTime = MPI_Wtime();

  if(myRank == 0) {
    if (yorn == 'y') {
      printf("sorted:\n");
      for(i = 0; i < P*size; i++) {
        printf("%f\n", dataSet[i]);
      }
    }
    printf("sorting took %f seconds.\n", endTime - startTime);
  }
  
  free(dataSet);
  free(localBuffer);
  free(thirdBuffer);
  
  MPI_Finalize();
  return 0;
}

/*
 * Just a thing.
 */
int argError(MPI_Comm comm) {
  MPI_Abort(comm, -1);
  return MPI_Finalize();
}

/* 
 * Comparator function, compares two doubles.
 */
int cmpfunc(const void * a, const void * b) {
  if (*(double*)a > *(double*)b) return 1;
  else if (*(double*)a < *(double*)b) return -1;
  return 0;
}

/* void sortedMerge
 * Takes two sorted arrays of length <size>, and merges them into the left array.
 */
void sortedMerge(double * left, double * right, int size) {
  int lp = 0;
  int rp = 0;
  int i = 0;
  
  double *tempArray = malloc(sizeof(double)*size*2); 

  while ( lp < size && rp < size) {
    if(left[lp] < right[rp]) { tempArray[i++] = left[lp++]; }
    else { tempArray[i++] = right[rp++]; }
  }

  while ( lp < size ) { tempArray[i++] = left[lp++]; }
  while ( rp < size ) { tempArray[i++] = right[rp++]; }

  memcpy(left, tempArray, sizeof(double)*size*2);
  free(tempArray);
  return;
}


void mergeLow(double *a, double *b, double *c, int n) {
  int countA = 0; int countB = 0; int countC = 0;
  while(countC < n) {
    if(cmpfunc(&a[countA],&b[countB])<0) {
      c[countC++] = a[countA++]; 
    } else {
      c[countC++] = b[countB++];
    }
  }
}

void mergeHigh(double *a, double *b, double *c, int n) {
  int countA = n-1; int countB = n-1; int countC = n-1;
  while(countC >= 0) {
    if(cmpfunc(&a[countA],&b[countB])>=0) {
      c[countC--] = a[countA--];
    } else {
      c[countC--] = b[countB--];
    }
  }
}
