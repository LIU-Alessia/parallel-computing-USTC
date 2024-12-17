#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <winsock2.h>  // 使用Windows上的等效头文件

int main(int argc, char *argv[]) {
    int rank, size, node_id;
    MPI_Comm node_comm;
    int root_rank = 0;  // 假设每个节点的根进程为“0号进程”
    char message[256];

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 获取节点编号，假设使用的是 SLURM 或其他支持环境变量的系统
    char hostname[256];
    DWORD hostname_len = sizeof(hostname);
    GetComputerNameA(hostname, &hostname_len);  // 使用GetComputerNameA函数替代GetComputerName
    node_id = rank;  

    // 将进程按节点分组
    MPI_Comm_split(MPI_COMM_WORLD, node_id, rank, &node_comm);

    // 输出每个节点中的进程信息
    int node_rank, node_size;
    MPI_Comm_rank(node_comm, &node_rank);
    MPI_Comm_size(node_comm, &node_size);
/*
    if (node_rank == 0) {
        // 每个节点的0号进程设置消息
        snprintf(message, sizeof(message), "Hello from node %d, root %d!", node_id, rank);
        printf("Node %d, Root Rank %d: Message = '%s'\n", node_id, rank, message);
    }
*/
    // 在每个节点内的进程通过 MPI_Bcast 接收消息
    MPI_Bcast(message, sizeof(message), MPI_CHAR, root_rank, node_comm);

    // 输出每个进程收到的消息
    printf("Rank %d, Node %d: Received message = '%s'\n", rank, node_id, message);

    MPI_Comm_free(&node_comm);
    MPI_Finalize();
    return 0;
}
