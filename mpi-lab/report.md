# <center>并行程序设计实验报告</center>

### <center>刘天润SC24219058</center>

# MPI-LAB

## 1 problem a
1.1）写个将 MPI 进程按其所在节点分组的程序;（1.2）在 1.1 的基础
上，写个广播程序，主要思想是：按节点分组后，广播的 root 进程将消息
“发送”给各组的“0 号”，再由这些“0”号进程在其小组内执行 MPI_Bcast。

解答过程的抽象如下：
![alt text](image-2.png)

### （1.1）进程分组
实验环境只有一个节点，在此直接使用 MPI_Comm_split 创建新的通信组。__MPI_Comm_split__ 的用法如下：
`MPI_Comm_split(MyWorld,Color,Key,&SplitWorld)`函数调用则在通信域MyWorld的基础上产生了几个分割的子通信域。原通信域MyWorld中的进程按照不同的Color值处在不同的分割通信域中，每个进程在不同分割通信域中的进程编号则由Key值来标识
![alt text](image-11.png)
```c
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
```

### （1.2）分层广播

```c
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

    // 组内广播消息
    MPI_Bcast(message, sizeof(message), MPI_CHAR, 0, group_comm);

    // 每个进程输出接收的消息
    printf("Rank %d (group %d, intra-group rank %d) received message: %s\n",
           rank, group_id, intra_group_rank, message);

    // 释放通信子
    MPI_Comm_free(&group_comm);

    MPI_Finalize();
    return 0;
```
运行结果如下：

![alt text](image-1.png)

注意这里要用 __非阻塞发送 (MPI_Isend)__ ，否则会造成死锁：
![alt text](image-13.png)

__原因分析：__ 全局 root 进程 (rank 0) 使用 MPI_Send 向每组的 root 发送消息，但是如果接收方（组内的 rank 0）没有准备好调用 MPI_Recv，发送方会阻塞等待，从而导致死锁。
使用 MPI_Isend 替代阻塞的 MPI_Send，并释放请求对象 (MPI_Request_free),
可以避免全局 root 进程等待接收方就绪的情况下发生死锁。(但是不让rank0给rank0发消息虽然不会死锁，但程序也会运行不了？：

![alt text](image-12.png)

## 2 problem b
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

## 3 problem c
N 个处理器求 N 个数的全和，要求每个处理器均保持全和。

### （1） 
蝶式全和的示意图如下： 由于使用了重复计算，共需 logN 步。给出蝶式全和计算的 MPI 程序实现（设 N 为 2 的幂次方）。
![alt text](image-4.png)
```c
int main(int argc, char *argv[]) {
    int rank, size, step, partner;
    int local_value, received_value;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 假设每个处理器持有的初始值是其 rank
    // 即计算0+1+2+3+4+5+6+7
    local_value = rank;

    // 蝶式全和过程 (logN 步)
    //每个处理器都发送消息给partner并接受来自partner的消息
    for (step = 0; step < (int)(log2(size)); step++) {
        partner = rank ^ (1 << step);  // 计算通信的处理器partner，编号相差 2的step次方的处理器会进行通信。分别跟0001,0010,0100异或
        printf("partner in step %d:%d\n ",step,partner);
        MPI_Sendrecv(&local_value, 1, MPI_INT, partner, 0, 
                     &received_value, 1, MPI_INT, partner, 0, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 更新处理器存储的值
        local_value += received_value;
    }

    // 每个处理器都有全和结果
    printf("Processor %d has total sum = %d\n", rank, local_value);

    MPI_Finalize();
    return 0;
}

```
运行结果：

![alt text](image-6.png)

解题说明：
1. 使用MPI_Sendrecv函数，每个处理器都发送消息给partner并接受来自partner的消息。下图以处理器processor 0为例，描述处理器在整个过程中的消息传递操作
   ![alt text](image-7.png)
2. 计算通信的处理器partner：根据蝶形运算的规律，编号相差 $2^{step}$的处理器会进行通信。可以通过异或操作实现，每个处理器分别跟0001,0010,0100异或。例如第1个处理器0001，异或的结果是0，0011，0101，即0，3，5。
   
### （2）
![alt text](image-8.png)
```c
int main(int argc, char *argv[]) {
    int rank, size, step;
    int local_value, received_value;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 假设每个处理器持有的初始值是其 rank
    local_value = rank;

    // 向上汇聚阶段 (logN 步)
    for (step = 1; step < size; step *= 2) {
        if (rank % (2 * step) == 0) {
            // 当前节点收集来自子节点的数据
            MPI_Recv(&received_value, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            local_value += received_value;
        } else if (rank % step == 0) {
            // 当前节点将数据发送给父节点
            MPI_Send(&local_value, 1, MPI_INT, rank - step, 0, MPI_COMM_WORLD);
            break;  // 子节点任务完成，退出
        }
    }

    // 向下广播阶段 (logN 步)
    for (step = size / 2; step >= 1; step /= 2) {
        if (rank % (2 * step) == 0) {
            // 当前节点向子节点发送数据
            MPI_Send(&local_value, 1, MPI_INT, rank + step, 0, MPI_COMM_WORLD);
        } else if (rank % step == 0) {
            // 当前节点接收来自父节点的数据
            MPI_Recv(&local_value, 1, MPI_INT, rank - step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    // 每个处理器都有全和结果
    printf("Processor %d has total sum = %d\n", rank, local_value);

    MPI_Finalize();
    return 0;
}
```
运行结果
![alt text](image-9.png)
解题说明：
1. 由题图，二叉树求和的方法：在每一步选择一个处理器储存求和的结果，向上传递汇总到一个处理器，再由这个处理器将求和结果依次传递给剩余处理器。两个过程分别需要$logN$步，故一共$2logN$。
2. 与碟式运算相比，除了第一步所有处理器发送数据外，不是所有处理器参与计算。处理器每次也不是和同一个处理器通信，而是接受子节点数据并发送数据给父节点处理器，因此需要分别使用MPI_Send和MPI_Recv通信。

## 4 problem d
![alt text](image-10.png)
