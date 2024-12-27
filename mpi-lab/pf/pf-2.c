#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 10  //假设矩阵大小为 10x10

int main(int argc, char *argv[]) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    float A[N][N], B[N][N];
    int i,j;
    //初始化矩阵 A,B
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            A[i][j]  = (float)(i+ j);
            B[i][j]=0;
        }
    }
    
    int block_size=(N-2) /(int)sqrt(size);
    int start_colmun, end_colmun;
    int start_row, end_row;
    //划分并行的block
    start_row = (int)(rank/(int)sqrt(size))*block_size+1;
    start_colmun=(int)(rank%(int)sqrt(size))*block_size+1;
    end_row=start_row+block_size-1;
    end_colmun=start_colmun+block_size-1;
    //printf("rank %d start_row %d end_row %d start_colmun %d end_colmun %d\n", rank, start_row, end_row, start_colmun, end_colmun);
    //分块计算
    for(i=start_row;i<=end_row;i++){
        for(j=start_colmun;j<=end_colmun;j++){
            B[i][j]=(A[i-1][j] + A[i][j+1] + A[i+1][j] + A[i][j-1]) / 4.0;
            //printf("rank %d B[%d][%d] %f\n", rank, i, j, B[i][j]);
        }
    }

    if(rank!=0)
    {
        float *senddata=(float *)malloc(sizeof(float)*block_size*block_size);
        int z=0;
        for(i=start_row;i<=end_row;i++){
            for(j=start_colmun;j<=end_colmun;j++){
                senddata[z]=B[i][j];
                z++;
                //printf("%d:%.2f ",z,senddata[z-1]);
            }
        }
        //发送结果到主进程
        MPI_Gather(senddata, block_size*block_size, MPI_FLOAT, NULL, 0, MPI_FLOAT, 0, MPI_COMM_WORLD);
        free(senddata);
    }
    if(rank==0)
    {
        //接收区要为size大小来接收包含根进程在内的所有数据，否则会报错
        float *recvdata=(float *)malloc(sizeof(float)*block_size*block_size*(size));
        //注意不能把这个接收代码放在循环中，否则会因重复接收而报错死锁
        MPI_Gather(NULL, 0, MPI_FLOAT, recvdata, block_size*block_size, MPI_FLOAT, 0, MPI_COMM_WORLD);
        //接受区数据放回到B中
        int index=block_size*block_size;
        for(int rank_i=1;rank_i<size;rank_i++)
        {   //循环接收来自于各个节点的数据
            start_row = (int)(rank_i/(int)sqrt(size))*block_size+1;
            start_colmun=(int)(rank_i%(int)sqrt(size))*block_size+1;
            for(i=start_row;i<=start_row+block_size-1;i++)
            {// 循环对指定位置的块赋值
                for(j=start_colmun;j<=start_colmun+block_size-1;j++){
                    //printf("i:%d,j:%d,%f ",i,j,recvdata[index]);
                    B[i][j]=recvdata[index];
                    index++;
                }
            }
        }
       free(recvdata);
    }

    // 打印结果
    if (rank == 0) {
        printf("Result matrix B:\n");
        for (i = 0; i < N; i++) {
            for (j = 0; j < N; j++) {
                printf("%.0f ", B[i][j]);
            }
            printf("\n");
        }
    }

    MPI_Finalize();
    return 0;
}