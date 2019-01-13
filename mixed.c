#include <stdio.h>
#include <stdlib.h>

//timing routine for reading the time stamp counter
static __inline__ unsigned long long rdtsc(void) {
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

void transpose(double *a, int lda, double *b, int ldb, int N){
  if (N <= 8) {
    int i, j;
    for (i = 0; i < N; ++i) {
      for (j = 0; j < N; j += 2) {
        *(b + ldb * j + i) = *(a + lda * i + j); 
        *(b + ldb * (j + 1) + i) = *(a + lda * i + (j + 1)); 
      }
    }
  }
  else{
    //Top-Left quadrant
    transpose(a,               lda, b, ldb, N/2);

    //Bottom-Left quadrant
    transpose(a+N/2,           lda, b+(N/2*ldb), ldb, N/2);

    //Bottom-Right quadrant
    transpose(a+(N/2*lda+N/2), lda, b+(N/2*lda+N/2), ldb, N/2);

    //Top-Right quadrant
    transpose(a+(N/2*lda),     lda, b+N/2, ldb, N/2);
  } 
}

void print_matrix(FILE *f, double *a, int N){
  int i , j;
  for (i = 0; i != N; ++i){
    for (j = 0; j != N; ++j){
      fprintf(f, "%3.3lf ", a[i*N+j]);
    }
    fprintf(f, "\n");
  }
  fprintf(f, "\n");
}

int main(int argc, char **argv){
  double *a, *b;

  int N = atoi(argv[1]);
  int runs = atoi(argv[2]);
  
  unsigned long long t0, t1, sum = 0;

  posix_memalign((void**)&a, 64, sizeof(double)*N*N);
  posix_memalign((void**)&b, 64, sizeof(double)*N*N);

  //initialize data
  int i = 0, j = 0;
  for (i = 0; i != N; ++i){
    for (j = 0; j != N; ++j){
      a[i*N+j] = (i+1)*1.0 + (j+1)*0.001;
      b[i] = 0.0;
    }
  }

  FILE *f = fopen("result.txt", "w");
  
  print_matrix(f, a, N);

  for(i = 0; i != runs; ++i) { 
    t0 = rdtsc();
    transpose(a, N, b, N, N);
    t1 = rdtsc();

    sum += (t1 - t0);
  }

  printf("Input size: %lu [B]\tExecution time: %lf [cycles]\n", N * N * sizeof(double), (double) (sum / (1.0 * runs)));

  print_matrix(f, b, N);

  free(a);
  free(b);
  return 0;
}
