#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define GROUP_SIZE 3 // 每组的大小

int main(int argc, char *argv[]) {
    int rank, size;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // 当前进程的 rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); // 总进程数

    // 确定有多少组
    int num_groups = (size + GROUP_SIZE - 1) / GROUP_SIZE; // 计算组数（向上取整）

    // 确定当前进程所属的组和组内的 rank
    int group_id = rank / GROUP_SIZE;         // 当前进程所在的组号
    int intra_group_rank = rank % GROUP_SIZE; // 当前进程在组内的 rank

    // 创建组内通信子
    MPI_Comm group_comm;
    MPI_Comm_split(MPI_COMM_WORLD, group_id, rank, &group_comm);

    // 获取组内 rank 和 size
    int group_rank, group_size;
    MPI_Comm_rank(group_comm, &group_rank);
    MPI_Comm_size(group_comm, &group_size);

    char message[50]; // 用于存储广播的消息
    int global_bcast_root = 0;

    // 全局 root 进程（rank 0）准备广播消息
    if (rank == global_bcast_root) {
        snprintf(message, sizeof(message), "Hello from rank 0!");
        printf("Global root (rank 0) broadcasting message to group roots...\n");

        // 非阻塞发送消息到每组的 root
        for (int i = 0; i < num_groups; i++) {
            int group_root_rank = i * GROUP_SIZE; // 每组的 root（组内 rank 0）在 MPI_COMM_WORLD 的 rank
            if (group_root_rank < size) { // 确保组存在
                MPI_Request request;
                MPI_Isend(message, sizeof(message), MPI_CHAR, group_root_rank, 0, MPI_COMM_WORLD, &request);
                MPI_Request_free(&request); // 释放请求，非阻塞发送不会阻塞全局 root
            }
        }
    }

    // 各组的 root (组内 rank 0) 接收消息
    if (intra_group_rank == 0) {
        MPI_Recv(message, sizeof(message), MPI_CHAR, global_bcast_root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Group %d root (rank %d in MPI_COMM_WORLD) received message: %s\n", group_id, rank, message);
    }
    // 同步：确保每组的 root 都接收到消息后再进行组内广播
    MPI_Barrier(MPI_COMM_WORLD);

    // 组内广播消息
    MPI_Bcast(message, sizeof(message), MPI_CHAR, 0, group_comm);

    // 每个进程输出接收的消息
    printf("Rank %d (group %d, intra-group rank %d) received message: %s\n",
           rank, group_id, intra_group_rank, message);

    // 释放通信子
    MPI_Comm_free(&group_comm);

    MPI_Finalize();
    return 0;
}