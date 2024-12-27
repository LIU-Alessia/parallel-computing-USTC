#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10
int sendCount[10];
int offset[10];
int main(int argc, char** argv) {
    int rank, size;
    double start_time,end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 矩阵A和B的分配
    double *A = (double *)malloc(N * N * sizeof(double));
    double *B = (double *)malloc(N * N * sizeof(double));
    memset(B, 0, sizeof(double)*N*N);
    
    if (rank == 0) {
        // 初始化矩阵A
        start_time=MPI_Wtime();
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i * N + j] = i+j; 
            }
        }
    }

    // 分配矩阵A到每个处理器
    int rows_per_proc = (N-2) / size;
    double *local_A = (double *)malloc((rows_per_proc+2) * N * sizeof(double));
    double *local_B = (double *)malloc(rows_per_proc * N * sizeof(double));
    for (int i = 0; i < size; i++) {
      sendCount[i] = (rows_per_proc+2) * N; //发送目标行及其上下两行给各个进程
      offset[i] = 1 + rows_per_proc * i;   //指定发送缓冲区中每个进程的偏移量
    }
    MPI_Scatterv(A,sendCount,offset, MPI_DOUBLE, local_A,(rows_per_proc+2)*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 计算
    for (int i = 1; i < rows_per_proc + 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            local_B[(i-1) * N + j] = (local_A[(i-1) * N + j] + local_A[i * N + (j+1)] + local_A[(i+1) * N + j] + local_A[i * N + (j-1)]) / 4.0;
        }
    }

    // 数据收集
    MPI_Gather(local_B, rows_per_proc * N, MPI_DOUBLE, B + N, rows_per_proc * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // 输出结果
    if (rank == 0) {
        end_time=MPI_Wtime();
        printf("main time:%f\n",end_time-start_time);
        }

    // 释放内存
    free(A);
    free(B);
    free(local_A);
    free(local_B);

    MPI_Finalize();
    return 1;
}
