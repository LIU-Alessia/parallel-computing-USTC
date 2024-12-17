# <center>并行程序设计实验报告</center>

### <center>刘天润SC24219058</center>

## MPI-LAB

### 1 problem a
1.1）写个将 MPI 进程按其所在节点分组的程序;（1.2）在 1.1 的基础
上，写个广播程序，主要思想是：按节点分组后，广播的 root 进程将消息
“发送”给各组的“0 号”，再由这些“0”号进程在其小组内执行 MPI_Bcast.

（1.1）使用 MPI_Comm_split 来基于每个进程的节点信息创建新的通信组

```c
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

    if (node_rank == 0) {
        // 每个节点的0号进程设置消息
        snprintf(message, sizeof(message), "Hello from node %d, root %d!", node_id, rank);
        printf("Node %d, Root Rank %d: Message = '%s'\n", node_id, rank, message);
    }

    MPI_Comm_free(&node_comm);
    MPI_Finalize();
    return 0;
}


```
运行结果如下：

![alt text](image-1.png)

（1.2）在上面基础上，加入广播功能，使得每个节点的“0 号”进程将消息发送给节点内的所有进程。每个节点内的进程通过 MPI_Bcast 接收消息

```c
 if (node_rank == 0) {
        // 每个节点的0号进程设置消息
        snprintf(message, sizeof(message), "Hello from node %d, root %d!", node_id, rank);
        printf("Node %d, Root Rank %d: Message = '%s'\n", node_id, rank, message);
    }

    // 在每个节点内的进程通过 MPI_Bcast 接收消息
    MPI_Bcast(message, sizeof(message), MPI_CHAR, root_rank, node_comm);

```
![alt text](image-2.png)

### 2 problem b
使用 MPI_Send 和 MPI_Recv 来模拟 MPI_Alltoall。将你的实验与相关 MPI通信函数做评测和对比。

```c
//自定义alltoall函数
void MPI_Alltoall_my(int* senddata, int sendcount, MPI_Datatype senddatatype, int* recvdata, int recvcount,
        MPI_Datatype recvdatatype, MPI_Comm comm) {
        int rank, size;
        MPI_Status status;
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);
        //每个进程都会遍历所有进程（包括自己）
        for (int i = 0; i < size; i++) {
            //如果当前进程 i 不是自己（i != rank），则使用 MPI_Send 发送数据到进程 i，并使用 MPI_Recv 从进程 i 接收数据。
            if (i != rank) {
                MPI_Send(senddata + i * sendcount, sendcount, senddatatype, i, rank , comm);
                MPI_Recv(recvdata + i * recvcount, recvcount, recvdatatype, i, i, comm, &status);            
            }
            //如果当前进程 i 是自己（i == rank），则直接将发送缓冲区的数据复制到接收缓冲区。 
            else {
                for (int j = i*sendcount; j < i*sendcount + sendcount; j++) {
                    recvdata[j] = senddata[j];
                }
            }
        }
}
```
对比自定义的Alltoall函数和MPI_Alltoall的性能
```c
int main() {
    int i, rank, datasize;
    datasize = 500;
    int size;
    double start_time, end_time;
    double min_start_time, max_end_time;
    //初始化 MPI 环境，获取当前进程的 rank 和总进程数
    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int *send, *recv;
    //分配发送和接收缓冲区，并初始化发送缓冲区的数据。
    send = (int*)malloc(sizeof(int)*datasize*size);
    recv = (int*)malloc(sizeof(int)*datasize*size);
    for (int j = 0; j < datasize*size; j++) {
        send[j] = j+1;
    }

    //测量自定义 MPI_Alltoall_my 函数的执行时间，并输出结果
    start_time = MPI_Wtime();
    MPI_Alltoall_my(send, datasize, MPI_INT, recv, datasize, MPI_INT, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    MPI_Reduce(&start_time, &min_start_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&end_time, &max_end_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("my alltoall time: %f\n", max_end_time - min_start_time);
    }
    //同步所有进程，确保精确测量时间
    MPI_Barrier(MPI_COMM_WORLD);

    //测量MPI_Alltoall函数的执行时间，并输出结果
    start_time = MPI_Wtime();
    MPI_Alltoall(send, datasize, MPI_INT, recv, datasize, MPI_INT, MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    MPI_Reduce(&start_time, &min_start_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&end_time, &max_end_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("MPI_alltoall total time: %f\n", max_end_time - min_start_time);
    }
    MPI_Finalize();
}
```
运行结果如下：
![alt text](image-3.png)
设置不同的进程数多次实验，得到性能加速图表如下：

| 进程数| 2  |4     |6     |     8|
|---|   ---|    ---|    ---|    ---|
|my |0.003642|0.008050|0.005132|0.007585
|mpi|0.000179|0.000271|0.000095|0.000108
|加速比|20.3|29.7|54.0|70.2

由上表可得，用MPI在进程间进行多对多通信时，直接用MPI_Send和MPI_Recv函数虽能实现基本功能，但耗费的时间非常多，效率显著低于MPI自带的方法MPI_Alltoall。加速比为模拟Alltoall的时间除以使用MPI_Alltoall的时间，由上表，随着互相通信的进程数增加，使用MPI_Alltoall的加速比会越来越大。
