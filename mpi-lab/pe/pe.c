#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include<time.h>

int main( int argc, char* argv[] ){
    int P=2;  //2个参数服务器进程
    int Q =6; //6个工作进程
    int rank, size;
    int senddate=0,sum=0,allsum=0;
    double average=0.0;
    double collect[10];
    int count=2;  //展示两轮互动过程
 
    MPI_Init( &argc, &argv );
    MPI_Status(status);
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );

    MPI_Comm Pcomm,Qcomm;
   //参数服务器组成子通信域Pcomm
    MPI_Comm_split(MPI_COMM_WORLD,rank/P,rank%P,&Pcomm); 
   //每个参数服务器与其对应的所有工作进程组成一个子通信域Qcomm
    MPI_Comm_split(MPI_COMM_WORLD,rank%P,rank/P,&Qcomm);
    //注意初始化随机数种子。time(NULL) 获取当前时间，rank 确保每个进程有不同的种子
    srand(time(NULL) + rank);
    for(int i=0;i<count;i++){
    if(rank>P-1){
         senddate=rand()%100;
         printf("process%d random%d:  %d\n",rank,i+1,senddate);
    }
//参数服务器对其对应工作进程产生的随机数进行求和归约
    MPI_Reduce(&senddate,&sum,1,MPI_INT,MPI_SUM,0,Qcomm); 
 //参数服务器间求和归约（每个都持有全和）
    MPI_Allreduce(&sum,&allsum,1,MPI_INT,MPI_SUM,Pcomm);
    average=(double)allsum/Q; //注意此处求均值是除以工作进程的数量
 //参数服务器将平均值广播给对应的工作进程
    MPI_Bcast(&average,1,MPI_DOUBLE,0,Qcomm);
    collect[i]=average;
    }

//结束输出平均值
    MPI_Barrier(MPI_COMM_WORLD);
    printf("process:%d average number:\t",rank);
    for(int i=0;i<count;i++){
      printf("%f\t",collect[i]);
    }
   printf("\n");
    MPI_Finalize();
    return 0;
}
