# <center>并行程序设计实验报告</center>

### <center>刘天润SC24219058</center>

## MPI-LAB

### 1 problem a
1.1）写个将 MPI 进程按其所在节点分组的程序;（1.2）在 1.1 的基础
上，写个广播程序，主要思想是：按节点分组后，广播的 root 进程将消息
“发送”给各组的“0 号”，再由这些“0”号进程在其小组内执行 MPI_Bcast.

使用 MPI_Comm_split 来基于每个进程的节点信息创建新的通信组

```c
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    int rank, size, node_id;
    MPI_Comm node_comm;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 获取节点编号，假设我们使用的是 SLURM 或其他支持环境变量的系统
    char hostname[256];
    gethostname(hostname, 256);
    node_id = rank;  // 为了简化假设每个进程一个节点

    // 将进程按节点分组
    MPI_Comm_split(MPI_COMM_WORLD, node_id, rank, &node_comm);

    // 输出每个节点中的进程信息
    int node_rank, node_size;
    MPI_Comm_rank(node_comm, &node_rank);
    MPI_Comm_size(node_comm, &node_size);

    printf("Rank %d, Node %d: Group size = %d, Node rank = %d\n", rank, node_id, node_size, node_rank);

    MPI_Comm_free(&node_comm);
    MPI_Finalize();
    return 0;
}

```

在上面基础上，加入广播功能，使得每个节点的“0 号”进程将消息发送给节点内的所有进程。每个节点内的进程通过 MPI_Bcast 接收消息

```c
 if (node_rank == 0) {
        // 每个节点的0号进程设置消息
        snprintf(message, sizeof(message), "Hello from node %d, root %d!", node_id, rank);
        printf("Node %d, Root Rank %d: Message = '%s'\n", node_id, rank, message);
    }

    // 在每个节点内的进程通过 MPI_Bcast 接收消息
    MPI_Bcast(message, sizeof(message), MPI_CHAR, root_rank, node_comm);

```
