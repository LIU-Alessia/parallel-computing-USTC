#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Tony Ladd算法实现MPI_Alltoall
void tony_ladd_alltoall(int *send_data, int *recv_data, int size, int rank) {
    int *buffer = (int *)malloc(size * sizeof(int)); // 缓存用于环形交换

    for (int step = 0; step < size; step++) {
        // 计算发送和接收的目标进程
        int send_to = (rank + step) % size;
        int recv_from = (rank - step + size) % size;

        if (step == 0) {
            //第一步直接拷贝自己的数据，无需通信
            for (int i = 0; i < size; i++) {
                buffer[i] = send_data[i];
            }
        } else {
            // 环形发送和接收
            MPI_Sendrecv(buffer, size, MPI_INT, send_to, 0,
                         recv_data, size, MPI_INT, recv_from, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // 更新缓存为接收到的数据
            for (int i = 0; i < size; i++) {
                buffer[i] = recv_data[i];
            }
        }
    }
    free(buffer);
}

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time; // 用于记录时间

    MPI_Init(&argc, &argv);              // 初始化 MPI 环境
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // 获取当前进程的 rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // 获取总进程数


    int *send_data = (int *)malloc(size * sizeof(int)); // 每个进程发送的数据
    int *recv_data = (int *)malloc(size * sizeof(int)); // 每个进程接收的数据

    // 初始化发送数据，每个进程发送给其它进程的数据为 rank*10 + i
    for (int i = 0; i < size; i++) {
        send_data[i] = rank * 10 + i;
    }

    // 记录开始时间
    start_time = MPI_Wtime();

    // 执行 Tony Ladd 的 Alltoall 算法
    tony_ladd_alltoall(send_data, recv_data, size, rank);

    // 记录结束时间
    end_time = MPI_Wtime();

    // 输出每个进程接收到的数据
    printf("Rank %d received data: ", rank);
    for (int i = 0; i < size; i++) {
        printf("%d ", recv_data[i]);
    }
    printf("\n");

    // 输出执行时间
    if (rank == 0) {
        printf("Tony Ladd Alltoall execution time: %f seconds\n", end_time - start_time);
    }

    // 释放内存
    free(send_data);
    free(recv_data);

    MPI_Finalize(); // 结束 MPI
    return 0;
}