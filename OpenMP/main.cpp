#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <xmmintrin.h>
#include <windows.h>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

const int N = 1024;
float mat[N][N], temp[N][N], result[N][N];

// int my_rank = omp_get_thread_num();
// int thread_count = omp_get_num_threads();
#ifdef _OPENMP
int my_rank = omp_get_thread_num();
//int thread_count = omp_get_num_threads();
int thread_count = 4;
#else
int my_rank = 0;
int thread_count = 1;
#endif

void print(float matrix[N][N]) //输出
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            cout << matrix[i][j] << " ";
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

void normal_gauss(float mat[N][N]) //普通高斯消去法
{
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
}

//OpenMP并行循环
void test1()
{
    int i,j;
    for (int k = 0; k < N; k++)
    {
        for (j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        #pragma omp parallel for num_threads(thread_count) shared(k,mat) private(i,j)
        for (i = k + 1; i < N; i++)
        {
            for (j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }
}

void test2()
{
#pragma omp parallel
    //cout << thread_count << endl;
    for (int k = 0; k < N; k++)
    {
        #pragma omp for
        for (int j = k + 1; j < N; j++)
        {
            mat[k][j] = mat[k][j] / mat[k][k];
        }
        mat[k][k] = 1.0;
        #pragma omp barrier

        #pragma omp for
        for (int i = k + 1; i < N; i++)
        {
            for (int l = k + 1; l < N; l++)
            {
                mat[i][l] = mat[i][l] - mat[i][k] * mat[k][l];
            }
            mat[i][k] = 0;
        }
        #pragma omp barrier
    }
}

int main()
{
    long long head, tail, freq; // timers
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 100;

    cout << "============= N=" << N << "============" << endl;

    copy(mat, temp);
    cout << "普通高斯消去" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 10; i++)
    {
        copy(mat, result);
        normal_gauss(result);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    //print(result);

    cout << endl;
    cout << "OpenMP + 循环划分" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    //for (int i = 0; i < 100; i++)
    //{
    copy(temp, mat);
    //#pragma omp parallel num_threads(thread_count)
    test1();
    //compare(result,mat);
    //}
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    //cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    //print(mat);

        cout << endl;
    cout << "OpenMP + 循环划分" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
    copy(temp, mat);
    //#pragma omp parallel num_threads(thread_count)
    test2();
    QueryPerformanceCounter((LARGE_INTEGER *)&tail);
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    //print(mat);
    return 0;
}
