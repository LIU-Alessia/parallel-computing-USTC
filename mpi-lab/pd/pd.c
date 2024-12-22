#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// 矩阵块乘法
void matrixMultiply(double *A, double *B, double *C, int blockSize) {
    for (int i = 0; i < blockSize; i++) {
        for (int j = 0; j < blockSize; j++) {
            for (int k = 0; k < blockSize; k++) {
                C[i * blockSize + j] += A[i * blockSize + k] * B[k * blockSize + j];
            }
        }
    }
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("size:%d\n",size);

    int sqrtP = (int)sqrt(size);  // 处理器网格维度
    int n = 8;                   // 矩阵维度（假设可被 sqrtP 整除）
    int blockSize = n / sqrtP;   // 子块大小

    // 每个处理器的 A, B 和 C 子块
    double *A_block = (double *)malloc(blockSize * blockSize * sizeof(double));
    double *B_block = (double *)malloc(blockSize * blockSize * sizeof(double));
    double *C_block = (double *)calloc(blockSize * blockSize, sizeof(double));

    // 初始化 A 和 B 子块（这里假设所有块已经分发）
    for (int i = 0; i < blockSize * blockSize; i++) {
        A_block[i] = rank + 1;  // 示例数据
        B_block[i] = rank + 1;
    }

    // 主计算循环
    for (int k = 0; k < sqrtP; k++) {
        // 确定广播的 A 的块的坐标
        int broadcastRoot = (rank / sqrtP) * sqrtP + k;
        double *A_temp = (double *)malloc(blockSize * blockSize * sizeof(double));

        // 广播 A 的块
        if (rank == broadcastRoot) {
            MPI_Bcast(A_block, blockSize * blockSize, MPI_DOUBLE, broadcastRoot, MPI_COMM_WORLD);
        } else {
            MPI_Bcast(A_temp, blockSize * blockSize, MPI_DOUBLE, broadcastRoot, MPI_COMM_WORLD);
        }

        // 矩阵乘法
        if (rank == broadcastRoot) {
            matrixMultiply(A_block, B_block, C_block, blockSize);
        } else {
            matrixMultiply(A_temp, B_block, C_block, blockSize);
        }

        // 循环移动 B 块
        int sendTo = (rank + sqrtP) % size;
        int recvFrom = (rank - sqrtP + size) % size;
        MPI_Sendrecv_replace(B_block, blockSize * blockSize, MPI_DOUBLE, sendTo, 0, recvFrom, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        free(A_temp);
    }

    // 输出计算结果
    if (rank == 0) {
        printf("Computation complete.\n");
    }

    free(A_block);
    free(B_block);
    free(C_block);
    MPI_Finalize();
    return 0;
}