#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define M 4096       // Rows in matrix A and matrix C
#define K 4096       // Columns in matrix A and Rows in matrix B
#define N 4096       // Columns in matrix B and matrix C
#define TILE_SIZE 16 // Tile size for cache optimization

int num_threads = 8;   // Number of threads
int **A, **B, **C_non_opt, **C_opt;  // Matrices

void *matrix_multiply_non_opt(void *arg) {
    int tid = *(int *)arg; // Thread ID

    int rows_per_thread = M / num_threads;
    int start_row = tid * rows_per_thread;
    int end_row = (tid + 1) * rows_per_thread;

    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < K; k++) {
                sum += A[i][k] * B[k][j];
            }
            C_non_opt[i][j] = sum;
        }
    }
    pthread_exit(0);
}

void *matrix_multiply_optimized(void *arg) {
    int tid = *(int *)arg; // Thread ID

    int rows_per_thread = M / num_threads;
    int start_row = tid * rows_per_thread;
    int end_row = (tid + 1) * rows_per_thread;

    for (int i = start_row; i < end_row; i += TILE_SIZE) {
        for (int j = 0; j < N; j += TILE_SIZE) {
            for (int k = 0; k < K; k += TILE_SIZE) {
                for (int ii = i; ii < i + TILE_SIZE && ii < M; ii++) {
                    for (int jj = j; jj < j + TILE_SIZE && jj < N; jj++) {
                        int sum = 0;
                        for (int kk = k; kk < k + TILE_SIZE && kk < K; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }
                        C_opt[ii][jj] += sum;
                    }
                }
            }
        }
    }
    pthread_exit(0);
}

int main() {
  // Allocate matrices
    A = malloc(M * sizeof(int *));
    B = malloc(K * sizeof(int *));
    C_non_opt = malloc(M * sizeof(int *));
    C_opt = malloc(M * sizeof(int *));
    for (int i = 0; i < M; i++) A[i] = malloc(K * sizeof(int));
    for (int i = 0; i < K; i++) B[i] = malloc(N * sizeof(int));
    for (int i = 0; i < M; i++) {
        C_non_opt[i] = malloc(N * sizeof(int));
        C_opt[i] = malloc(N * sizeof(int));
    }

    // Initialize matrices with random values
    for (int i = 0; i < M; i++)
        for (int j = 0; j < K; j++)
            A[i][j] = rand() % 10;

    for (int i = 0; i < K; i++)
        for (int j = 0; j < N; j++)
            B[i][j] = rand() % 10;

    // Initialize result matrices
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            C_non_opt[i][j] = 0;
            C_opt[i][j] = 0;
        }
    }

    pthread_t threads[num_threads];
    int thread_ids[num_threads];


    struct timeval start, end;
    double elapsed_time_non_opt, elapsed_time_opt;

    // Run non-optimized version
    gettimeofday(&start, NULL);
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, matrix_multiply_non_opt, &thread_ids[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed_time_non_opt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    // Run optimized version
    gettimeofday(&start, NULL);
    for (int i = 0; i < num_threads; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, matrix_multiply_optimized, &thread_ids[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    gettimeofday(&end, NULL);
    elapsed_time_opt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;

    // Print results
    printf("Non-optimized version time: %.2f ms\n", elapsed_time_non_opt);
    printf("Optimized version time: %.2f ms\n", elapsed_time_opt);

    // Free allocated memory
    for (int i = 0; i < M; i++) {
        free(A[i]);
        free(C_non_opt[i]);
        free(C_opt[i]);
    }
    for (int i = 0; i < K; i++) free(B[i]);
    free(A);
    free(B);
    free(C_non_opt);
    free(C_opt);
    

    return 0;
}
