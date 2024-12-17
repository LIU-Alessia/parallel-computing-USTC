
% datasize=500时不同进程数
comm_size=[1,2,4,6,8]
my=[0.006650,0.005727,0.007537,0.010132,0.001835];
mpi=[0.000068,0.000109,0.000115,0.000407,0.000146];
rate=my./mpi;
% 8个进程不同数据大小
datasize=[500,1000,1500,2000];
my_alltoall_8=[0.001835,0.008909,0.008407,1];
mpi_8=[0.000146,0.0011];
plot(comm_size,rate);
%plot(datasize,my_alltoall);
legend('my_alltoall','mpi_alltoall')