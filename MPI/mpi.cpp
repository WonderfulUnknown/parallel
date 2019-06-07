#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"

using namespace std;

const int N = 1024;
float mat[N][N], temp[N][N], answer[N][N], result[N][N];
int counts, my_id;

void print(float mat[N][N]) //输出
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            cout << mat[i][j] << " ";
        cout << endl;
    }
}

void compare(float m1[N][N], float m2[N][N])
{
    bool flag = true;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            if (m1[i][j] != m2[i][j])
                flag = false;
    }
    cout << "======================" << endl;
    if (flag)
        cout << "the result is the same" << endl;
    else
        cout << "the result is different" << endl;
    cout << "======================" << endl;
}

void copy(float m1[N][N], float m2[N][N])
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            m2[i][j] = m1[i][j];
    }
}

void normal_gauss() //普通高斯消去法
{
    srand(time(0));
    struct timeval tstart, tend;
    gettimeofday(&tstart, NULL);
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }

    gettimeofday(&tend, NULL);
    cout << "普通高斯消去: " << (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000 << " ms" << endl;
}

int main(int argc, char *argv[])
{
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);  //报告识别调用进程的rank
    MPI_Comm_size(MPI_COMM_WORLD, &counts); //报告进程数
    if (my_id == 0)
    {
        srand((unsigned)time(NULL));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                mat[i][j] = rand() % 100;

        cout << "============= N=" << N << "============" << endl;

        copy(mat, temp);
        normal_gauss();
        copy(mat, answer);
        /*print(answer);*/
        copy(temp, mat);
    }

    float msg[N];
    int block_size = N / counts;

    srand(time(0));
    struct timeval tstart, tend;
    gettimeofday(&tstart, NULL);
    for (int k = 0; k < N; k++)
    {
        if (my_id == 0)
        {
            /*0号进程把矩阵传递给其他进程*/
            for (int i = 1; i < counts; i++)
                for (int j = i * (N / counts); j < (i + 1) * (N / counts); j++)
                    MPI_Send(mat[j], N, MPI_FLOAT, i, 99, MPI_COMM_WORLD);
        }
        else
        {
            for (int i = 0; i < N / counts; i++)
                MPI_Recv(mat[i], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD, &status);
        }
    }

    int m = 0;
    for (int k = 0; k < N; k++)
    {
        if (k < (my_id + 1) * block_size && k >= my_id * block_size)
        {
            for (int j = k + 1; j < N; j++)
                mat[m][j] = mat[m][j] / mat[m][k];
            mat[m][k] = 1;
            for (int j = 0; j < N; j++)
                msg[j] = mat[m][j];
            m++; //保证按序，需要考虑是否有其他更好的写法
        }
        /*将计算好的结果广播*/
        MPI_Bcast(msg, N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        if (k < (my_id + 1) * block_size)
        {
            for (int i = m; i < block_size; i++)
            {
                for (int j = k + 1; j < N; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * msg[j];
                mat[i][k] = 0;
            }
            MPI_Bcast(mat[k], N, MPI_FLOAT, 0, MPI_COMM_WORLD);
        }
    }
    if (my_id == 0)
    {
        gettimeofday(&tend, NULL);
        cout << endl;
        cout << "MPI块划分: " << (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000 << " ms" << endl;
        compare(answer, mat);
        /*print(mat);*/
    }

    MPI_Finalize();
    return 0;
}
