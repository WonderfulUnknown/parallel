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

const int N = 512;
float mat[N][N], temp[N][N], result[N][N];

#ifdef _OPENMP
int my_rank = omp_get_thread_num();
int thread_count = 4;
#else
int my_rank = 0;
int thread_count = 1;
#endif

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

//OpenMP循环划分
void test1()
{
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        #pragma omp parallel for
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }
}

//OpenMP循环划分,4,5行并行
void test2()
{
    #pragma omp parallel
    for (int k = 0; k < N; k++)
    {
        #pragma omp for
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        #pragma omp for
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }
}

//OpenMP循环划分，4,5行并行, SSE
void test3()
{
    __m128 t1, t2, t3;
    float a[4];
    #pragma omp parallel private(t1,t2,t3,a)
    for (int k = 0; k < N; k++)
    {
        #pragma omp for
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        #pragma omp for
        for (int i = k + 1; i < N; i++)
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

//OpenMP动态划分
void test4()
{
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        #pragma omp parallel for schedule(dynamic, N / thread_count)
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
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
    for (int i = 0; i  < 10; i++)
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
    for (int i = 0; i  < 10; i++)
    {
        copy(temp, mat);
        test1();
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    compare(result, mat);

    cout << endl;
    cout << "OpenMP + 循环划分 + 动态调度" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
    for (int i = 0; i  < 10; i++)
    {
        copy(temp, mat);
        test4();
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    compare(result, mat);

    cout << endl;
    cout << "OpenMP + 循环划分 + 4,5行并行" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
    for (int i = 0; i  < 10; i++)
    {
        copy(temp, mat);
        test2();
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    compare(result, mat);

    cout << endl;
    cout << "OpenMP + 循环划分 + 4,5行并行 + SSE" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);
    for (int i = 0; i  < 10; i++)
    {
        copy(temp, mat);
        test3();
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    compare(result, mat);

    return 0;
}
