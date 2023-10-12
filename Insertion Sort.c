#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MAX_NUMBER 1000

void insertionSort(int arr[], int n) {
    int i, key, j;
    for (i = 1; i < n; i++) {
        key = arr[i];
        j = i - 1;
        
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int main(int argc, char* argv[]) {
    int my_rank;      /* rank of process      */
    int p;            /* number of processes  */
    int source;       /* rank of sender       */
    int dest;         /* rank of receiver     */
    int tag = 0;      /* tag for messages     */
    int* arr = NULL;  /* storage for numbers  */
    MPI_Status status; /* return status for receive */

    /* Start up MPI */
    MPI_Init(&argc, &argv);

    /* Find out process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int i; // Declare loop variable outside of the for loops

    if (my_rank == 0) {
        /* Create random numbers */
        arr = (int*)malloc(p * sizeof(int));

        srand(time(NULL) + my_rank); // Seed the random number generator with a changing value
        for (i = 0; i < p; i++) {
            arr[i] = rand() % MAX_NUMBER;
        }

        /* Print the numbers before sorting */
        printf("The generated numbers are:\n");
        for (i = 0; i < p; i++) {
            printf("%4d", arr[i]);
            if ((i + 1) % 10 == 0) printf("\n");
        }
        printf("\n");
    }

    // Broadcast the number of elements to be sorted
    MPI_Bcast(&p, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (my_rank != 0) {
        arr = (int*)malloc(p * sizeof(int)); // Allocate memory for other processes
    }

    // Distribute data to other processes in the pipeline
    MPI_Bcast(arr, p, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform insertion sort
    insertionSort(arr, p);

    // Pass sorted data in the pipeline (forward)
    if (my_rank != 0) {
        MPI_Send(arr, p, MPI_INT, my_rank - 1, tag, MPI_COMM_WORLD);
    }

    // Pass sorted data in the pipeline (backward)
    if (my_rank != p - 1) {
        MPI_Recv(arr, p, MPI_INT, my_rank + 1, tag, MPI_COMM_WORLD, &status);
    }

    // Process 0 collects the sorted numbers and prints them
    if (my_rank == 0) {
        printf("The sorted numbers are:\n");
        for (i = 0; i < p; i++) {
            printf("%4d", arr[i]);
            if ((i + 1) % 10 == 0) printf("\n");
        }
        printf("\n");
    }

    /* Shut down MPI */
    MPI_Finalize();

    return 0;
}
