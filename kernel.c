#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

unsigned long long rdtsc()
{
    unsigned long long int x;
    unsigned a, d;
    __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
    return ((unsigned long long) a) | (((unsigned long long) d) << 32);
}


#define loadBack(dest, row, m256row) \
    *(dest + (row * 8))  = m256row[0]; \
    *(dest + (row * 8 + 1))  = m256row[1]; \
    *(dest + (row * 8 + 2))  = m256row[2]; \
    *(dest + (row * 8 + 3))  = m256row[3]; \


#define kernel(A, B, crow1, crow2, crow3, crow4, crow5, crow6, \
        crow7, crow8, crow9, crow10, crow11, crow12, \
        cast1, cast2, cast3, cast4, cast5, cast6, loadB, loadB2) \
    cast1 = _mm256_broadcast_sd(A + 0); \
    cast2 = _mm256_broadcast_sd(A + 1); \
    cast3 = _mm256_broadcast_sd(A + 2); \
    cast4 = _mm256_broadcast_sd(A + 3); \
    cast5 = _mm256_broadcast_sd(A + 4); \
    cast6 = _mm256_broadcast_sd(A + 5); \
    loadB1 = _mm256_load_pd(B); \
    loadB2 = _mm256_load_pd(B + 4); \
    crow1 = _mm256_fmadd_pd(cast1, loadB1, crow1); \
    crow2 = _mm256_fmadd_pd(cast2, loadB1, crow2); \
    crow3 = _mm256_fmadd_pd(cast3, loadB1, crow3); \
    crow4 = _mm256_fmadd_pd(cast4, loadB1, crow4); \
    crow5 = _mm256_fmadd_pd(cast5, loadB1, crow5); \
    crow6 = _mm256_fmadd_pd(cast6, loadB1, crow6); \
    crow7 = _mm256_fmadd_pd(cast1, loadB1, crow7); \
    crow8 = _mm256_fmadd_pd(cast2, loadB1, crow8); \
    crow9 = _mm256_fmadd_pd(cast3, loadB1, crow9); \
    crow10 = _mm256_fmadd_pd(cast4, loadB1, crow10); \
    crow11 = _mm256_fmadd_pd(cast5, loadB1, crow11); \
    crow12 = _mm256_fmadd_pd(cast6, loadB1, crow12); \

// ---
// This function will do matrix multiply for a 6 x K
// and K x 8 matrix and store the result to a 6 x 8 
// C matrix.
// ---
void matrixMultiply(double *A, double *B, double *C, int k)
{
    int i, j;
    __m256d crow1, crow2, crow3, crow4, crow5, crow6;
    __m256d crow7, crow8, crow9, crow10, crow11, crow12;
    __m256d cast1, cast2, cast3, cast4, cast5, cast6;

    for (int i = 0; i < 4; ++i) {
        crow1[i] = 0.0; crow2[i] = 0.0; crow3[i] = 0.0; 
        crow4[i] = 0.0; crow5[i] = 0.0; crow6[i] = 0.0; 
        crow7[i] = 0.0; crow8[i] = 0.0; crow9[i] = 0.0; 
        crow10[i] = 0.0; crow11[i] = 0.0; crow12[i] = 0.0; 
    }

    __m256d loadB1, loadB2; // B1 handles the first 4 doubles and B2 last 4 of a row of B
    for (int i = 0; i < k; ++i) {
        kernel(A + i * 6, B + i * 8, crow1, crow2, crow3, crow4, crow5, crow6, crow7, crow8, crow9, crow10, crow11, crow12, cast1, cast2, cast3, cast4, cast5, cast6, loadB1, loadB2);
    }

    loadBack(C, 0, crow1);
    loadBack(C, 1, crow2);
    loadBack(C, 2, crow3);
    loadBack(C, 3, crow4);
    loadBack(C, 4, crow5);
    loadBack(C, 5, crow6);
    loadBack(C + 4, 0, crow7);
    loadBack(C + 4, 1, crow8);
    loadBack(C + 4, 2, crow9);
    loadBack(C + 4, 3, crow10);
    loadBack(C + 4, 4, crow11);
    loadBack(C + 4, 5, crow12);
}


void print_matrix(FILE* f, double *a, int rows, int columns)
{
    int i, j;
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < columns; ++j) {
            fprintf(f, "%3.3lf ", a[i * columns + j]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "\n");
}


int main(int argc, char** argv)
{
    int k = atoi(argv[1]);
    int runs = atoi(argv[2]);
    int mc = 16;

    double *A, *B, *C;
    unsigned long long t0, t1, sum = 0;

    posix_memalign((void**)&A, 64, sizeof(double)*16*k*6);
    posix_memalign((void**)&B, 64, sizeof(double)*k*8);
    posix_memalign((void**)&C, 64, sizeof(double)*16*6*8);

    int i, j;
    for (i = 0; i < k; ++i) {
        for (j = 0; j < 96; ++j) {
            A[i * 96 + j] = (i + 1) * 1.0 + (j + 1) * 0.001; 
        }
    }
    for (i = 0; i < k; ++i) {
        for (j = 0; j < 8; ++j) {
            B[i * 8 + j] = (j + 1) * 1.0 + (i + 1) * 0.001;
        }
    } 
    for (i = 0; i < 96; ++i) {
        for (j = 0; j < 8; ++j) {
            C[i * 8 + j] = 0.0;
        }
    }

    FILE* f = fopen("matrix_mul.txt", "w");

    print_matrix(f, A, k, 96);
    print_matrix(f, B, k, 8);

    for (j = 0; j < runs; ++j) {
        t0 = rdtsc();
        for (i = 0; i < mc; ++i) {
            matrixMultiply((A + (i * 6 * k)), B, (C + (i * 6 * 8)), k);
        }
        t1 = rdtsc();

        sum += (t1 - t0);
    }

    printf("K = %d \t Execution time: %lf [cycles]\n", k, (double) (sum / (1.0 * runs)));
    printf("Size of A: %lu [B]\n", 16 * k * 6 * sizeof(double));
    printf("Size of B: %lu [B]\n", 8 * k * sizeof(double));
    printf("Size of C: %lu [B]\n", 96 * 8 * sizeof(double));

    print_matrix(f, C, 96, 8);

    free(A);
    free(B);
    free(C);
    return (0);
}

