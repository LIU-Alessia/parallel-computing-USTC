#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10  // 矩阵大小

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time,end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int rows_per_process = N / size;
    int start_row = rank * rows_per_process;
    int end_row = (rank + 1) * rows_per_process;

    // 分配局部矩阵（包括边界行）
    double *local_A = malloc((rows_per_process + 2) * N * sizeof(double));
    double *local_B = malloc(rows_per_process * N * sizeof(double));

    // 初始化矩阵A（仅在主进程中进行）
    double A[N][N], B[N][N];
    if (rank == 0) {
          start_time=MPI_Wtime();
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                A[i][j] = i + j;  // 初始化A为行列索引的和，便于检验正确性
            }
        }
    }
    
    // 主进程分发数据
    if (rank == 0) {
        for (int p = 1; p < size; p++) {
          // 每个进程接受划分行及其上下两行
            MPI_Send(&A[p * rows_per_process - 1][0], (rows_per_process + 2) * N, MPI_DOUBLE, p, 0, MPI_COMM_WORLD);
        }
        // 主进程保留自己的数据
        for (int i = 0; i < rows_per_process + 2; i++) {
            for (int j = 0; j < N; j++) {
                local_A[i * N + j] = A[start_row + i - 1][j];
            }
        }
    } else {
        MPI_Recv(local_A, (rows_per_process + 2) * N, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // 并行计算
    for (int i = 1; i < rows_per_process + 1; i++) {
        for (int j = 1; j < N - 1; j++) {
            local_B[(i - 1) * N + j] = (local_A[(i - 1) * N + j] + local_A[i * N + (j + 1)] +
                                        local_A[(i + 1) * N + j] + local_A[i * N + (j - 1)]) / 4.0;
        }
    }

    // 收集结果
    if (rank == 0) {
        for (int i = 0; i < rows_per_process; i++) {
            for (int j = 0; j < N; j++) {
                B[start_row + i][j] = local_B[i * N + j];
            }
        }
        for (int p = 1; p < size; p++) {
            MPI_Recv(&B[p * rows_per_process][0], rows_per_process * N, MPI_DOUBLE, p, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    } else {
        MPI_Send(local_B, rows_per_process * N, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
    if (rank == 0) {
        end_time=MPI_Wtime();
        printf("main time:%f\n",end_time-start_time);}

    // // 主进程输出结果
    // if (rank == 0) {
    //     end_time=MPI_Wtime();
    //     printf("main time:%f\n",end_time-start_time);
    //     printf("Matrix A:\n");
    //     for (int i = 0; i < N; i++) {
    //         for (int j = 0; j < N; j++) {
    //             printf("%.0f ", A[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    // if (rank == 0) {
    //     printf("Matrix B:\n");
    //     for (int i = 0; i < N; i++) {
    //         for (int j = 0; j < N; j++) {
    //             printf("%.0f ", B[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    free(local_A);
    free(local_B);
    MPI_Finalize();
    return 0;
}