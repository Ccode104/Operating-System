#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define N 1024 // Matrix size

// Matrices
int A[N][N], B[N][N], C[N][N];
int BLOCK_SIZE;
int NUM_THREADS;
pthread_mutex_t lock;

// Struct to pass data to threads
typedef struct {
    int rowStart, colStart;
} BlockData;

void* blockMultiply(void* arg) {
    BlockData* data = (BlockData*)arg;
    int rowStart = data->rowStart;
    int colStart = data->colStart;
    for (int i = rowStart; i < rowStart + BLOCK_SIZE && i < N; i++) {
        for (int j = colStart; j < colStart + BLOCK_SIZE && j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    free(data);
    return NULL;
}


double parallelMatrixMultiply(int blockSize, int numThreads) {
    BLOCK_SIZE = blockSize;
    NUM_THREADS = numThreads;
    pthread_t threads[NUM_THREADS];
    int threadCount = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < N; i += BLOCK_SIZE) {
        for (int j = 0; j < N; j += BLOCK_SIZE) {
            if (threadCount < NUM_THREADS) {
                BlockData* data = (BlockData*)malloc(sizeof(BlockData));
                data->rowStart = i;
                data->colStart = j;
                pthread_create(&threads[threadCount++], NULL, blockMultiply, data);
            } else {
                for (int t = 0; t < NUM_THREADS; t++) {
                    pthread_join(threads[t], NULL);
                }
                threadCount = 0;
            }
        }
    }

    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

int main() {
    // Initialize matrices
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = 1;
            B[i][j] = 1;
            C[i][j] = 0;
        }
    }

    pthread_mutex_init(&lock, NULL);

    FILE *fp = fopen("results.csv", "w");
    fprintf(fp, "Threads,BlockSize,Time\n");

    int threadCounts[] = {1 ,2, 4, 8, 16, 32};
    int blockSizes[] = {8, 16, 32, 64};

    for (int i = 0; i < sizeof(threadCounts) / sizeof(threadCounts[0]); i++) {
        for (int j = 0; j < sizeof(blockSizes) / sizeof(blockSizes[0]); j++) {
            double time_taken = parallelMatrixMultiply(blockSizes[j], threadCounts[i]);
            fprintf(fp, "%d,%d,%f\n", threadCounts[i], blockSizes[j], time_taken);
            printf("Threads: %d, Block Size: %d, Time: %f\n", threadCounts[i], blockSizes[j], time_taken);
        }
    }

    fclose(fp);
    pthread_mutex_destroy(&lock);

    printf("Results saved to results.csv\n");
    return 0;
}


