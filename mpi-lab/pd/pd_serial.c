#include <stdio.h>
#include <time.h>

//FOX矩阵乘法的串行实现

int main()
{
  double begintime,endtime;
  begintime=clock();
  int n=4;
    int a[4][4], b[4][4], c[4][4];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            a[i][j] =n*i+j;
              b[i][j]= n * i + j;
        }
    }
    printf("\nMatrix a:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          printf("%d\t",a[i][j]);
        }
        printf( "\n");
    
    }
    printf("\nMatrix b:\n");
    
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          printf("%d\t",b[i][j]);
        }
        printf( "\n");
    
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                c[i][j] = 0;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                c[i][j] +=a[i][k] * b[k][j];
            }
        } 
    }
    printf("\nMatrix a*b:\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
          printf("%d\t",c[i][j]);
        }
        printf( "\n");
    
    }
   endtime=clock();
   printf("Running time:%fs\n",(endtime-begintime)/CLOCKS_PER_SEC);
    return 0;
}


