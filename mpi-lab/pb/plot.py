import matplotlib.pyplot as plt

# 数据
processes = [2, 4, 6, 8]
my_mpi_speedup = [20.3, 29.7, 54.0, 70.2]
ladd_speedup = [1.35, 1.94, 8.49, 12.36]

# 绘制图表
plt.plot(processes, my_mpi_speedup, marker='o', label='my_mpi')
plt.plot(processes, ladd_speedup, marker='s', label='Ladd')

# 添加标题和标签
plt.title('Speedup Comparison: my_mpi vs Ladd (vs MPI_Alltoall)')
plt.xlabel('Number of Processes')
plt.ylabel('Speedup (Method Time / MPI_Alltoall Time)')
plt.legend()

# 显示图表
plt.grid(True)
plt.show()