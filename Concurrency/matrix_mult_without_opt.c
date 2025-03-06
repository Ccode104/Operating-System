#include<stdio.h>
#include<stdlib.h> //For malloc
#include<time.h>

//K is the shared dimension
#define M 1024
#define K 1024
#define N 1024

int main()
{
	//Declare the matrices(Segmentation fault!!)
	//int A[M][K],B[K][N],C[M][N];

	//Use dynamic allocation
	int **A,**B,**C;
	A=(int**)malloc(sizeof(int*)*M);
	B=(int**)malloc(sizeof(int*)*K);
	C=(int**)malloc(sizeof(int*)*M);

	for(int i=0;i<M;i++)
		A[i]=(int*)malloc(sizeof(int)*K);
	for(int i=0;i<K;i++)
		B[i]=(int*)malloc(sizeof(int)*N);
	for(int i=0;i<M;i++)
		C[i]=(int*)malloc(sizeof(int)*N);


	//Initialise the matrix C to 0
	for(int i=0;i<M;i++)
		for(int j=0;j<N;j++)
				C[i][j]=0;
	//Initialise the matrix A to 1
	for(int i=0;i<M;i++)
		for(int j=0;j<K;j++)
				A[i][j]=1;

	//Initialise the matrix B to 1
	for(int i=0;i<K;i++)
		for(int j=0;j<N;j++)
				B[i][j]=1;

	struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

	//A*B=C
	for(int i=0;i<M;i++)
		for(int j=0;j<N;j++)
			for(int k=0;k<K;k++)
				C[i][j]+=A[i][k]*B[k][j];

	//In the above arrangement of loops, we multiply ith row of A with jth column of B and store it in C[i][j]

	clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9; // in seconds
	
	/*
	//Print C to verify output
	for(int i=0;i<M;i++)
	{
		printf("\n");
		for(int j=0;j<N;j++)
				printf(" %d ",C[i][j]);
	}
	*/

	printf("\nTime taken : %lf seconds \n",time_taken);


}