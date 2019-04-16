#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>

const int N = 8;

using namespace std;

void normal_gaosi(float matrix[N][N]) //普通高斯消去法
{
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            matrix[k][j] = matrix[k][j] / matrix[k][k];
        matrix[k][k] = 1.0;
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

void SSE_gaosi(float matrix[N][N]) //SSE并行化
{
    __m128 t1, t2, t3;
    float temp[4];
    for (int k = 0; k < N; k++)
    {
        for(int x = 0;x < 4; x++)
            temp[x] = matrix[k][k];
        t1 = _mm_loadu_ps(temp);
        for (int j = N - 4; j >= k; j -= 4)
        {
            t2 = _mm_loadu_ps(matrix[k] + j);
            t3 = _mm_div_ps(t2, t1); //除法
            _mm_storeu_ps(matrix[k] + j, t3);
        }
        if (k % 4 != (N % 4)) //处理k不能被4整除的元素
        {
            // for (int j = k; j % 4 != (N % 4); j++)
            for (int j = k; j % 4; j++)
                matrix[k][j] = matrix[k][j] / temp[0];
        }
        // for (int j = (N % 4) - 1; j >= 0; j--) //考虑N不能被4整除的情况
        //     matrix[k][j] = matrix[k][j] / temp[0];
        for (int i = k + 1; i < N; i++)
        {
            for (int x = 0; x < 4; x++)
                temp[x] = matrix[i][k];
            t1 = _mm_loadu_ps(temp);
            for (int j = N - 4; j > k; j -= 4)
            {
                t2 = _mm_loadu_ps(matrix[i] + j);
                t3 = _mm_loadu_ps(matrix[k] + j);
                t3 = _mm_mul_ps(t1,t3);
                t3 = _mm_sub_ps(t2, t3);
                _mm_storeu_ps(matrix[i] + j, t3);
            }
            // for (int j = k + 1; j % 4 != (N % 4); j++)
            for (int j = k + 1; j % 4; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

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

int main()
{
    long long head, tail, freq; // timers
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    float matrix[N][N], copy[N][N];
    srand((unsigned)time(NULL));
    cout << "生成随机矩阵" << endl;
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            matrix[i][j] = rand() % 100;
            copy[i][j] = matrix[i][j];
        }
    }
    print(matrix);

    cout << endl;
    cout << "普通高斯消去法" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    normal_gaosi(matrix);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时： " << (tail - head) * 1000.0 / freq << "ms" << endl;
    print(matrix);

    cout << endl;
    cout << "使用SSE并行" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    SSE_gaosi(copy);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时： " << (tail - head) * 1000.0 / freq << "ms" << endl;
    print(copy);

    compare(matrix, copy);
    system("pause");
    return 0;
}
