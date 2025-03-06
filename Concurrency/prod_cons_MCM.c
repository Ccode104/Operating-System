#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MATRIX_SIZE 10 // Define matrix dimensions

// Global variables for producer-consumer
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_not_full = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER;

typedef struct {
    int **data;
    int rows;
    int cols;
} Matrix;

typedef struct {
    Matrix *matA;
    Matrix *matB;
} MatrixPair;

// Dynamic buffer size and counters
MatrixPair *buffer;
int buffer_size;
int buffer_count = 0;
int buffer_in = 0;
int buffer_out = 0;
int done = 0; // Flag for termination condition

Matrix *generate_random_matrix(int rows, int cols) {
    Matrix *mat = malloc(sizeof(Matrix));
    if (!mat) return NULL;
    mat->rows = rows;
    mat->cols = cols;
    mat->data = malloc(rows * sizeof(int*));
    if (!mat->data) {
        free(mat);
        return NULL;
    }

    for (int i = 0; i < rows; ++i) {
        mat->data[i] = malloc(cols * sizeof(int));
        if (!mat->data[i]) {
            for (int j = 0; j < i; ++j) free(mat->data[j]);
            free(mat->data);
            free(mat);
            return NULL;
        }
        for (int j = 0; j < cols; ++j) {
            mat->data[i][j] = rand() % 10;
        }
    }
    return mat;
}

void free_matrix(Matrix *mat) {
    if (mat) {
        for (int i = 0; i < mat->rows; ++i) {
            free(mat->data[i]);
        }
        free(mat->data);
        free(mat);
    }
}

// Producer function
void *producer(void *arg) {
    int id = *(int *)arg;
    while (!done) {
        Matrix *matA = generate_random_matrix(MATRIX_SIZE, MATRIX_SIZE);
        Matrix *matB = generate_random_matrix(MATRIX_SIZE, MATRIX_SIZE);

        pthread_mutex_lock(&mtx);

        // Wait if buffer is full
        while (buffer_count == buffer_size) {
            printf("Producer %d waiting - buffer full.\n", id);
            pthread_cond_wait(&buffer_not_full, &mtx);
        }

        // Insert matrices into the buffer
        buffer[buffer_in].matA = matA;
        buffer[buffer_in].matB = matB;
        buffer_in = (buffer_in + 1) % buffer_size;
        buffer_count++;
        printf("Producer %d produced a matrix pair.\n", id);

        // Signal consumers that buffer is not empty
        pthread_cond_signal(&buffer_not_empty);
        pthread_mutex_unlock(&mtx);

        sleep(1); // Optional sleep for pacing
    }
    return NULL;
}

// Matrix multiplication function
Matrix *multiply_matrices(Matrix *A, Matrix *B) {
    Matrix *C = generate_random_matrix(A->rows, B->cols);
    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < B->cols; j++) {
            C->data[i][j] = 0;
            for (int k = 0; k < A->cols; k++) {
                C->data[i][j] += A->data[i][k] * B->data[k][j];
            }
        }
    }
    return C;
}

// Consumer function
void *consumer(void *arg) {
    int id = *(int *)arg;
    while (!done) {
        pthread_mutex_lock(&mtx);

        // Wait if buffer is empty
        while (buffer_count == 0) {
            printf("Consumer %d waiting - buffer empty.\n", id);
            pthread_cond_wait(&buffer_not_empty, &mtx);
        }

        // Retrieve matrices from the buffer
        Matrix *matA = buffer[buffer_out].matA;
        Matrix *matB = buffer[buffer_out].matB;
        buffer_out = (buffer_out + 1) % buffer_size;
        buffer_count--;
        printf("Consumer %d retrieved a matrix pair and started multiplication.\n", id);

        // Signal producers that buffer is not full
        pthread_cond_signal(&buffer_not_full);
        pthread_mutex_unlock(&mtx);

        // Perform matrix multiplication
        Matrix *result = multiply_matrices(matA, matB);
        
        // Free matrices after use
        free_matrix(matA);
        free_matrix(matB);
        free_matrix(result);

        sleep(1); // Optional sleep for pacing
    }
    return NULL;
}

void run_test(int num_producers, int num_consumers, int buf_size) {
    buffer_size = buf_size;
    buffer = malloc(buffer_size * sizeof(MatrixPair));

    pthread_t producers[num_producers], consumers[num_consumers];
    int producer_ids[num_producers], consumer_ids[num_consumers];

    for (int i = 0; i < num_producers; ++i) {
        producer_ids[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &producer_ids[i]);
    }

    for (int i = 0; i < num_consumers; ++i) {
        consumer_ids[i] = i + 1;
        pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]);
    }

    sleep(10); // Simulate running time for each test
    done = 1;

    // Signal all consumers to wake up and exit
    pthread_cond_broadcast(&buffer_not_empty);

    for (int i = 0; i < num_producers; ++i)
        pthread_join(producers[i], NULL);
    for (int i = 0; i < num_consumers; ++i)
        pthread_join(consumers[i], NULL);

    free(buffer);
}

int main() {
    int num_producers = 3;
    int num_consumers = 3;
    int buf_size = 5;

    run_test(num_producers, num_consumers, buf_size);

    printf("Tests completed.\n");
    return 0;
}

