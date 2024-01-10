#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct sortingParams_struct {
  double *ptr;
  int size;
} sortingParams;

typedef struct mergingParams_struct {
  double *first;
  int firstsize;
  double *second;
  int secondsize;
  double *dest;
} mergingParams;

void swap(double* p, int x, int y){
  double temp;
  
  temp = p[y];
  p[y] = p[x];
  p[x] = temp;
}


void * selectionSort(void *arg){
  sortingParams *p = arg;
  int currMinIndex;

  for (int i = 0; i < p->size; i++){
    currMinIndex = i;

    for (int j = i+1; j < p->size; j++){
      if (p->ptr[j] < p->ptr[currMinIndex]){
        currMinIndex = j; 
      }
    }
    if (currMinIndex != i){
      swap(p->ptr, i, currMinIndex);
    }
  }
}

void * mergeThreads(void *arg){
  //merge two sorted arrays into a third array
    mergingParams *merge = arg;
    int firstindex = 0;
    int secondindex = 0;
    for (int i = 0; i < (merge->firstsize + merge->secondsize); i++)    {
      if (firstindex == merge->firstsize && secondindex == merge->secondsize){
        exit(0);
      } else if (firstindex == merge->firstsize){
        merge->dest[i] = merge->second[secondindex];
        secondindex++;
      } else if (secondindex == merge->secondsize){
        merge->dest[i] = merge->first[firstindex];
        firstindex++;
      } else {
        if (merge->first[firstindex] < merge->second[secondindex]){
          merge->dest[i] = merge->first[firstindex];
          firstindex++;
        } else if (merge->second[secondindex] <= merge->first[firstindex]){
          merge->dest[i] = merge->second[secondindex];
          secondindex++;
        }
      }
    }
  }

double rand_double(){

  int rand_int;
  double rand_double;

  rand_int = rand() % 10000;
  rand_double = rand_int / 10.0;
  
  return rand_double;
}



void printArray(sortingParams* p) {
  // takes a pointer to a struct and prints the array
  
  for (int i = 0; i < p->size-1; i++){
    printf("[%.1f],", p->ptr[i]);
  }
  printf("[%.1f]\n", p->ptr[p->size-1]);
}


int main(int argc, char *argv[]) {

  int n;
  if (argc == 2){

    sscanf(argv[1], "%d", &n);

  } else {
    printf("Must specify array size\n");
    return 0;
  }

  struct timespec ts_begin, ts_end;
	double elapsed;

  
  //create array A (n double values) and randomly generate these values 
  double a[n];
  for (int i = 0; i < n; i++){
    a[i] = rand_double();
  }
  
  //also create arrays B and C with the same size of A
  double b[n];
  double c[n];

  //create firstHalf and secondHalf with the half size of A
  double firstHalf[n/2];
  double secondHalf[n - (n/2)];

  
/*
/////////////////////////////////
Begin One Thread Case
/////////////////////////////////
*/

  //copy A into B
  for (int i = 0; i < n; i++){
    b[i] = a[i];
  }

  clock_gettime(CLOCK_MONOTONIC, &ts_begin);

  //package params for sorting function
  sortingParams bInfo;
  double *pb = &b[0];
  bInfo.size = n;
  bInfo.ptr = pb;
  
  //create threadB sortThread to sort B
  pthread_t threadB;
  pthread_create(&threadB, NULL, selectionSort, (void *) &bInfo);
   //join threadB
  pthread_join(threadB, NULL);

  //printArray(&bInfo);
  
  clock_gettime(CLOCK_MONOTONIC, &ts_end);  
	elapsed = ts_end.tv_sec - ts_begin.tv_sec; 
	elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0; 
 
  printf("Sorting by ONE thread is done in %.1f ms\n", elapsed * 1000);


  /**
  ////////////////////////////////
    TWO THREADS CASE
  ////////////////////////////////
  **/

  //copy A into firstHalf  and secondHalf
  for (int i = 0; i < n; i++){
    if (i < n/2){
      firstHalf[i] = a[i];
    } else {
      secondHalf[i - (n/2)] = a[i];
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &ts_begin); 

  //package up params into structs
  sortingParams fhInfo;
  double *pf = &firstHalf[0];
  fhInfo.size = sizeof(firstHalf)/(sizeof(firstHalf[0]));
  fhInfo.ptr = pf;

  sortingParams shInfo;
  double *ps = &secondHalf[0];
  shInfo.size = sizeof(secondHalf)/(sizeof(secondHalf[0]));
  shInfo.ptr = ps;

  

  pthread_t threadA1;
  pthread_create(&threadA1, NULL, selectionSort, (void *) &fhInfo);

  pthread_t threadA2;
  pthread_create(&threadA2, NULL, selectionSort, (void *) &shInfo);

  pthread_join(threadA1, NULL);
  pthread_join(threadA2, NULL);

  //printArray(&fhInfo);
  //printArray(&shInfo);

  // c for merge destination
  sortingParams cInfo;
  double *pc = &c[0];
  cInfo.size = n;
  cInfo.ptr = pc;

  mergingParams merge;
  merge.first = fhInfo.ptr;
  merge.second = shInfo.ptr;
  merge.firstsize = n/2;
  merge.secondsize = n - (n/2);
  merge.dest = cInfo.ptr;

  pthread_t threadM;
  pthread_create(&threadM, NULL, mergeThreads, (void *) &merge);

  pthread_join(threadM, NULL);

 // printArray(&cInfo);

  clock_gettime(CLOCK_MONOTONIC, &ts_end);  
	elapsed = ts_end.tv_sec - ts_begin.tv_sec; 
	elapsed += (ts_end.tv_nsec - ts_begin.tv_nsec) / 1000000000.0;

  printf("Sorting by TWO threads is done in %.1f ms\n", elapsed * 1000);
  
  return 0;
  
}
