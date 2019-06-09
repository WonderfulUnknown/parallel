#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "mpi.h"
#include <xmmintrin.h> //SSE
#include <emmintrin.h> //SSE2
#include <pmmintrin.h> //SSE3
#include <tmmintrin.h> //SSSE3
#include <smmintrin.h> //SSE4.1
#include <nmmintrin.h> //SSSE4.2

using namespace std;

#ifdef _OPENMP
#include <omp.h>
#endif

#ifdef _OPENMP
int my_rank = omp_get_thread_num();
int thread_count = 4;
#else
int my_rank = 0;
int thread_count = 1;
#endif

const int N = 512;
float mat[N][N], temp[N][N], answer[N][N];
int counts, my_id;
long long head, tail, freq; // timers

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
    // srand(time(0));
    // struct timeval tstart, tend;
    // gettimeofday(&tstart, NULL);
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
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

    // gettimeofday(&tend, NULL);
    // cout << "普通高斯消去: " << (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000 << " ms" << endl;
    if (my_id == 0)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
        cout << "普通高斯消去：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    }
}

/*MPI块划分*/
void test1()
{
    MPI_Status status;
    if (my_id == 0)
        copy(temp, mat);
    /*把矩阵广播给所有进程*/
    MPI_Bcast(mat, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int block_size = N / counts;
    int begin = my_id * block_size;
    int end = (my_id + 1) * block_size;
    int root = 0;

    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time

    for (int k = 0; k < N; k++)
    {
        root = k / block_size;
        if (k >= begin && k < end)
        {
            for (int j = k + 1; j < N; j++)
                mat[k][j] = mat[k][j] / mat[k][k];
            mat[k][k] = 1;
        }
        /*将自己计算好的结果广播*/
        MPI_Bcast(mat[k], N, MPI_FLOAT, root, MPI_COMM_WORLD);

        for (int i = k + 1; i < N; i++)
        {
            //root = i / block_size;
            if (i >= begin && i < end)
            {
                for (int j = k + 1; j < N; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
                mat[i][k] = 0;
            }
            //不必要的传播，几乎相当于串行
            // MPI_Bcast(mat[i], N, MPI_FLOAT, root, MPI_COMM_WORLD);
            // if (my_id == root && my_id != 0)
            // MPI_Send(mat[i], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
            // else if (my_id != root && my_id == 0)
            // MPI_Recv(mat[i], N, MPI_FLOAT, root, 99, MPI_COMM_WORLD, &status);
        }
    }
    if (my_id != 0)
    {
        for (int j = begin; j < end; j++)
            MPI_Send(mat[j], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
    }
    else
    {
        for (int i = 1; i < counts; i++)
            MPI_Recv(mat[i * block_size], block_size * N, MPI_FLOAT, i, 99, MPI_COMM_WORLD, &status);
        QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
        cout << endl;
        cout << "MPI块划分：" << (tail - head) * 1000.0 / freq << "ms" << endl;
        compare(answer, mat);
    }
}

/*MPI循环划分*/
void test2()
{
    MPI_Status status;
    if (my_id == 0)
        copy(temp, mat);
    MPI_Bcast(mat, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int root = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time

    for (int k = 0; k < N; k++)
    {
        if (my_id == 0)
        {
            for (int j = k + 1; j < N; j++)
                mat[k][j] = mat[k][j] / mat[k][k];
            mat[k][k] = 1;
        }
        MPI_Bcast(mat[k], N, MPI_FLOAT, 0, MPI_COMM_WORLD);

        for (int i = k + 1; i < N; i++)
        {
            root = (i - k - 1) % counts;
            if (my_id == root)
            {
                for (int j = k + 1; j < N; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
                mat[i][k] = 0;
                if(my_id != 0)
                    MPI_Send(mat[i], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
            }
            // MPI_Bcast(mat[i], N, MPI_FLOAT, root, MPI_COMM_WORLD);
            // if (my_id == root && my_id != 0)
            //     MPI_Send(mat[i], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
            if (root != 0 && my_id == 0)
                MPI_Recv(mat[i], N, MPI_FLOAT, root, 99, MPI_COMM_WORLD, &status);
        }
    }

    if (my_id == 0)
    {
        QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
        cout << endl;
        cout << "循环划分：" << (tail - head) * 1000.0 / freq << "ms" << endl;
        compare(answer, mat);
    }
}

/*MPI块划分 + SSE*/
void test3()
{
    MPI_Status status;
    if (my_id == 0)
        copy(temp, mat);
    MPI_Bcast(mat, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int block_size = N / counts;
    int begin = my_id * block_size;
    int end = (my_id + 1) * block_size;
    int root = 0;

    __m128 t1, t2, t3;
    float a[4];

    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time

    for (int k = 0; k < N; k++)
    {
        root = k / block_size;
        if (k >= begin && k < end)
        {
            for (int j = k + 1; j < N; j++)
                mat[k][j] = mat[k][j] / mat[k][k];
            mat[k][k] = 1;
        }
        MPI_Bcast(mat[k], N, MPI_FLOAT, root, MPI_COMM_WORLD);

        for (int i = k + 1; i < N; i++)
        {
            if (i >= begin && i < end)
            {
                for (int x = 0; x < 4; x++)
                    a[x] = mat[i][k];
                t1 = _mm_loadu_ps(a);
                for (int j = N - 4; j > k; j -= 4)
                {
                    t2 = _mm_loadu_ps(mat[i] + j);
                    t3 = _mm_loadu_ps(mat[k] + j);
                    t3 = _mm_mul_ps(t1, t3);
                    t3 = _mm_sub_ps(t2, t3);
                    _mm_storeu_ps(mat[i] + j, t3);
                }
                for (int j = k + 1; j % 4; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
                mat[i][k] = 0;
            }
        }
    }
    if (my_id != 0)
    {
        for (int j = begin; j < end; j++)
            MPI_Send(mat[j], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
    }
    else
    {
        for (int i = 1; i < counts; i++)
            MPI_Recv(mat[i * block_size], block_size * N, MPI_FLOAT, i, 99, MPI_COMM_WORLD, &status);

        QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
        cout << endl;
        cout << "块划分+SSE：" << (tail - head) * 1000.0 / freq << "ms" << endl;
        compare(answer, mat);
    }
}

/*MPI循环划分 + openmp*/
void test4()
{
    MPI_Status status;
    if (my_id == 0)
        copy(temp, mat);
    /*把矩阵广播给所有进程*/
    MPI_Bcast(mat, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int block_size = N / counts;
    int begin = my_id * block_size;
    int end = (my_id + 1) * block_size;
    int root = 0;

    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time

    
    for (int k = 0; k < N; k++)
    {
        root = k / block_size;
        if (k >= begin && k < end)
        {
            for (int j = k + 1; j < N; j++)
                mat[k][j] = mat[k][j] / mat[k][k];
            mat[k][k] = 1;
        }
        MPI_Bcast(mat[k], N, MPI_FLOAT, root, MPI_COMM_WORLD);

        //#pragma omp parallel
        for (int i = k + 1; i < N; i++)
        {
            if (i >= begin && i < end)
            {
               //#pragma omp for
                for (int j = k + 1; j < N; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
                mat[i][k] = 0;
            }
        }
    }
    if (my_id != 0)
    {
        for (int j = begin; j < end; j++)
            MPI_Send(mat[j], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD);
    }
    else
    {
        for (int i = 1; i < counts; i++)
            MPI_Recv(mat[i * block_size], block_size * N, MPI_FLOAT, i, 99, MPI_COMM_WORLD, &status);
        QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
        cout << endl;
        cout << "MPI块划分：" << (tail - head) * 1000.0 / freq << "ms" << endl;
        compare(answer, mat);
    }
}

int main(int argc, char *argv[])
{
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    /*MPI_Status status;*/
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
    }

    test1();
    MPI_Barrier(MPI_COMM_WORLD);
    test2();
    MPI_Barrier(MPI_COMM_WORLD);
    test3();
    MPI_Barrier(MPI_COMM_WORLD);
    test4();

    MPI_Finalize();
    return 0;
}
