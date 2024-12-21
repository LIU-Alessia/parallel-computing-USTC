#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
    int rank, size, step, partner;
    int local_value, received_value;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // 假设每个处理器持有的初始值是其 rank
    //即计算0+1+2+3+4+5+6+7
    local_value = rank;

    // 蝶式全和过程 (logN 步)
    //每个处理器都发送消息给partner并接受来自partner的消息
    for (step = 0; step < (int)(log2(size)); step++) {
        partner = rank ^ (1 << step);  // 计算通信的处理器partner
        printf("partner in step %d:%d\n ",step,partner);
        MPI_Sendrecv(&local_value, 1, MPI_INT, partner, 0, 
                     &received_value, 1, MPI_INT, partner, 0, 
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // 更新本地值
        local_value += received_value;
    }

    // 每个处理器都有全和结果
    printf("Processor %d has total sum = %d\n", rank, local_value);

    MPI_Finalize();
    return 0;
}
