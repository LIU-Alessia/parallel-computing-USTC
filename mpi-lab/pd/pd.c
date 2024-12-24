#include <mpi.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

const int n=4;      //方阵的行列数
const int q=2;      //每行每列划分的线程数
const int m=2;      //每个线程持有方阵的行列数

void Mutply(int a[m][m],int b[m][m],int c[m][m]){  /*矩阵乘法*/
  for (int i=0;i<m;i++){
    for (int j=0;j<m;j++){
      for (int k=0;k<m;k++){
        c[i][j]=c[i][j]+a[i][k]*b[k][j];}
    }
  }
}

void copy(int x[m][m],int y[m][m]){     /*矩阵复制*/
  for (int i=0;i<m;i++){
    for(int j=0;j<m;j++){
      y[i][j]=x[i][j];}
  }
}

int main(int argc, char *argv[])
{
int rank;
int row,column,rowRank,columnRank;
int a[m][m],b[m][m],c[m][m],buffer[m][m];
double begintime,endtime;
begintime=clock();

MPI_Init(&argc, &argv);
MPI_Status(status);
MPI_Comm rowComm,columnComm;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);

row=rank/q;
column=rank%q;

MPI_Comm_split(MPI_COMM_WORLD,row,column,&rowComm);//行子通信域，每次迭代对矩阵A块MPI_Bcast广播

MPI_Comm_split(MPI_COMM_WORLD,column,row,&columnComm);//列子通信域，将矩阵B块MPI_Send发送给上一个线程，并MPI_Recv接收来自下一个线程的B块
MPI_Barrier(MPI_COMM_WORLD);
MPI_Comm_rank(rowComm,&rowRank);
MPI_Comm_rank(columnComm,&columnRank);

for (int i=0;i<m;i++){
  for (int j=0;j<m;j++){
    a[i][j]=(row*m+i)*n+column*m+j;
    b[i][j]=(row*m+i)*n+column*m+j;
    c[i][j]=0;
  }
}

for(int k=0;k<q;k++){
  if (column==(row+k)%q){
    copy(a,buffer);
  }
  MPI_Bcast(buffer,m*m,MPI_INT,(row+k)%q,rowComm);
  Mutply(buffer,b,c);
  copy(b,buffer);
  MPI_Send(buffer,m*m,MPI_INT,(columnRank-1+q)%q,1,columnComm);
  MPI_Recv(b,m*m,MPI_INT,(columnRank+1)%q,1,columnComm,&status);
  MPI_Barrier(MPI_COMM_WORLD);
}
printf("Proccess:%d\n",rank);
for(int i=0;i<m;i++){
  for(int j=0;j<m;j++){
    printf("%d\t",c[i][j]);
  }
  printf("\n");
}
MPI_Barrier(MPI_COMM_WORLD);

if(rank==0){
  endtime=clock();
  printf("Running time:%fs\n",(endtime-begintime)/CLOCKS_PER_SEC);
}

MPI_Finalize();
return 0;
}
