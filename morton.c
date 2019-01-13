#include <stdio.h>
#include <stdlib.h>


// timing routine for reading the time stamp counter
static __inline__ unsigned long long rdtsc(void) {
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

//// ######### REFERENCE ###########
//	HIGH PERFORMANCE PARALLELISM PEARLS - James Reinders, James Jeffers
int dilate_1(int x) {
    x = (x ^ (x << 8)) & 0x00ff00ff;
    x = (x ^ (x << 4)) & 0x0f0f0f0f;
    x = (x ^ (x << 2)) & 0x33333333;
    x = (x ^ (x << 1)) & 0x55555555;
    return (x);
}


int zindex2d (int column, int row)
{
    return ((dilate_1(row) << 1) | dilate_1(column));
}
/// ############### END REFERENCE ###########################


// ---
// I am having some memory penalty while storing zidxA and zidxB,
// Actually only N values are needed, but just to keep the code
// consistent, I have used up N*N space. This can be cut down definetly.
// ----
void transpose(double *a, int control, double *b, int N, int* zidxA, int* zidxB)
{
    if (N == 2) {
        *(b + *zidxB) = *(a + *zidxA);
        *(b + *zidxB + 1) = *(a + *zidxA + 2);
        *(b + *zidxB + 2) = *(a + *zidxA + 1);
        *(b + *zidxB + 3) = *(a + *zidxA + 3); 
    }
    else {
        transpose(a, control, b, N / 2, zidxA, zidxB);
        transpose(a, control, b, N / 2, zidxA + N / 2, zidxB + N / 2);
        transpose(a, control, b, N / 2, zidxA + (N / 2 * control + N / 2), zidxB + (N / 2 * control + N / 2));
        transpose(a, control, b, N / 2, zidxA + (N / 2 * control), zidxB + (N / 2 * control));
    }
}

void print_matrix(FILE* f, double* A, int N)
{
    int i, j;
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            fprintf(f, "%3.3lf ", A[i * N + j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}

// Print the order in which the Z ordering is done.
// Uncomment the print_matrix calls from the main function to test this.
void print_int_matrix(FILE* f, int * A, int N)
{
    int i, j;
    printf(" ###### Z - ORDERING DONE IN THE FOLLOWING WAY ######\n");
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            fprintf(f, "%d ", A[i * N + j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}


int main(int argc, char**argv) 
{
    int N = atoi(argv[1]);
    int runs = atoi(argv[2]);

    double *a, *b;
    int *zidxA, *zidxB;
    unsigned long long t0, t1, sum = 0;
    int i , j, idxA, idxB;

    posix_memalign((void**)&a, 64, sizeof(double)*N*N);
    posix_memalign((void**)&b, 64, sizeof(double)*N*N);
    posix_memalign((void**)&zidxA, 32, sizeof(int)*N*N);
    posix_memalign((void**)&zidxB, 32, sizeof(int)*N*N);

    // initialize data in morton Z order
    for (i = 0; i < N; ++i) {
        for (j = 0; j < N; ++j) {
            idxA = zindex2d(j, i);
            idxB = zindex2d(i, j);
            *(zidxA + N*i + j) = idxA;
            *(zidxB + N*i + j) = idxB;
            a[idxA] = (i + 1) * 1.0 + (j + 1) * 0.001;
            b[i] = 0.0;
        }
    }

    FILE* f = fopen("result.txt", "w");
    print_matrix(f, a, N);
//    print_int_matrix(f, zidxA, N);
//    print_int_matrix(f, zidxB, N);

    for (i = 0; i != runs; ++i) {
        t0 = rdtsc();
        transpose(a, N, b, N, zidxA, zidxB);
        t1 = rdtsc();
        sum += (t1 - t0);
    }

    printf("Input size: %lu [B]\tExecution time: %lf [cycles]\n", N * N * sizeof(double), (double) (sum / (1.0 * runs)));

    print_matrix(f, b, N); 

    free(a);
    free(b);
    free(zidxA);
    free(zidxB);
    return (0);
}
