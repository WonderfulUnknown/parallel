#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>

const int N = 256;

using namespace std;

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

void compare_d(float **m1, float **m2)
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

void copy_d(float **m1, float **m2)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            m2[i][j] = m1[i][j];
    }
}

void normal_gauss(float matrix[N][N]) //普通高斯消去法
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

void normal_gauss_d(float **matrix) //普通高斯消去法
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

//test1 4,5行未并行化 SSE 静态数组
void test1(float matrix[N][N])
{
    __m128 t1, t2, t3;
    float temp[4];
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            matrix[k][j] = matrix[k][j] / matrix[k][k];
        matrix[k][k] = 1.0;
        for (int i = k + 1; i < N; i++)
        {
            for (int x = 0; x < 4; x++)
                temp[x] = matrix[i][k];
            t1 = _mm_loadu_ps(temp);
            for (int j = N - 4; j > k; j -= 4)
            {
                t2 = _mm_loadu_ps(matrix[i] + j);
                t3 = _mm_loadu_ps(matrix[k] + j);
                t3 = _mm_mul_ps(t1, t3);
                t3 = _mm_sub_ps(t2, t3);
                _mm_storeu_ps(matrix[i] + j, t3);
            }
            for (int j = k + 1; j % 4; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

//test2 4,5行并行化 SSE 静态数组
void test2(float matrix[N][N])
{
    __m128 t1, t2, t3;
    float temp[4];
    for (int k = 0; k < N; k++)
    {
        for (int x = 0; x < 4; x++)
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
                t3 = _mm_mul_ps(t1, t3);
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

//test3 4,5行并行化 SSE 动态数组 未对齐
void test3(float **matrix)
{
    __m128 t1, t2, t3;
    float temp[4];
    for (int k = 0; k < N; k++)
    {
        for (int x = 0; x < 4; x++)
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
            for (int j = k; j % 4; j++)
                matrix[k][j] = matrix[k][j] / temp[0];
        }
        for (int i = k + 1; i < N; i++)
        {
            for (int x = 0; x < 4; x++)
                temp[x] = matrix[i][k];
            t1 = _mm_loadu_ps(temp);
            for (int j = N - 4; j > k; j -= 4)
            {
                t2 = _mm_loadu_ps(matrix[i] + j);
                t3 = _mm_loadu_ps(matrix[k] + j);
                t3 = _mm_mul_ps(t1, t3);
                t3 = _mm_sub_ps(t2, t3);
                _mm_storeu_ps(matrix[i] + j, t3);
            }
            for (int j = k + 1; j % 4; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

//test4 4,5行并行化 SSE 动态数组 对齐
void test4(float **matrix)
{
    __m128 t1, t2, t3;
    __declspec(align(16)) float temp[4];
    for (int k = 0; k < N; k++)
    {
        for (int x = 0; x < 4; x++)
            temp[x] = matrix[k][k];
        t1 = _mm_load_ps(temp);
        for (int j = N - 4; j >= k; j -= 4)
        {
            t2 = _mm_load_ps(matrix[k] + j);
            t3 = _mm_div_ps(t2, t1); //除法
            _mm_store_ps(matrix[k] + j, t3);
        }
        if (k % 4 != (N % 4)) //处理k不能被4整除的元素
        {
            for (int j = k; j % 4; j++)
                matrix[k][j] = matrix[k][j] / temp[0];
        }
        for (int i = k + 1; i < N; i++)
        {
            for (int x = 0; x < 4; x++)
                temp[x] = matrix[i][k];
            t1 = _mm_load_ps(temp);
            int aligned;
            if ((k + 1) % 4)
                aligned = (k + 1) - (k + 1) % 4 + 4;
            else
                aligned = (k + 1) - (k + 1) % 4;
            for (int j = k + 1; j < aligned; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            for (int j = N - 4; j > aligned; j -= 4)
            {
                t2 = _mm_load_ps(matrix[i] + j);
                t3 = _mm_load_ps(matrix[k] + j);
                t3 = _mm_mul_ps(t1, t3);
                t3 = _mm_sub_ps(t2, t3);
                _mm_store_ps(matrix[i] + j, t3);
            }
            for (int j = aligned; j % 4; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

//test5 4,5行并行化 AVX 动态数组 未对齐
void test5(float **matrix)
{
    __m256 t1, t2, t3;
    float temp[8];
    for (int k = 0; k < N; k++)
    {
        for (int x = 0; x < 8; x++)
            temp[x] = matrix[k][k];
        t1 = _mm256_loadu_ps(temp);
        for (int j = N - 8; j >= k; j -= 8)
        {
            t2 = _mm256_loadu_ps(matrix[k] + j);
            t3 = _mm256_div_ps(t2, t1); //除法
            _mm256_storeu_ps(matrix[k] + j, t3);
        }
        if (k % 8 != (N % 8)) //处理k不能被8整除的元素
        {
            for (int j = k; j % 8; j++)
                matrix[k][j] = matrix[k][j] / temp[0];
        }
        for (int i = k + 1; i < N; i++)
        {
            for (int x = 0; x < 8; x++)
                temp[x] = matrix[i][k];
            t1 = _mm256_loadu_ps(temp);
            for (int j = N - 8; j > k; j -= 8)
            {
                t2 = _mm256_loadu_ps(matrix[i] + j);
                t3 = _mm256_loadu_ps(matrix[k] + j);
                t3 = _mm256_mul_ps(t1, t3);
                t3 = _mm256_sub_ps(t2, t3);
                _mm256_storeu_ps(matrix[i] + j, t3);
            }
            for (int j = k + 1; j % 8; j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

int main()
{
    long long head, tail, freq; // timers
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    float matrix[N][N], temp[N][N];
    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            matrix[i][j] = rand() % 100;
    //print(matrix);

    cout << endl;
    cout << "普通高斯消去 静态数组" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy(matrix, temp);
        normal_gauss(matrix);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    //print(matrix);

    cout << endl;
    cout << "4,5行未并行化 SSE 静态数组" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy(matrix, temp);
        test1(temp);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    //print(temp);
    compare(matrix, temp);

    cout << endl;
    cout << "4,5行并行化 SSE 静态数组" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy(matrix, temp);
        test1(temp);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    compare(matrix, temp);
    //==================动态数组部分========================
    int n = 1024;
    float **mat = new float *[n];
    float **tmp = new float *[n];
    for (int i = 0; i < n; i++)
    {
        mat[i] = new float[n];
        tmp[i] = new float[n];
    }
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            mat[i][j] = rand() % 100;

    cout<<endl;
    cout<<"============= n=" << n<<"============"<<endl;

    cout << endl;
    cout << "普通高斯消去 动态数组" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
        normal_gauss_d(mat);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;

    copy_d(mat, tmp);
    cout << endl;
    cout << "4,5行并行化 SSE 动态数组 未对齐" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy_d(mat, tmp);
        test3(tmp);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100<< "ms" << endl;
    compare_d(mat, tmp);

    cout << endl;
    cout << "4,5行并行化 SSE 动态数组 对齐" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy_d(mat, tmp);
        test4(tmp);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100<< "ms" << endl;
    compare_d(mat, tmp);

    copy_d(mat, tmp);
    cout << endl;
    cout << "4,5行并行化 AVX 动态数组 未对齐" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy_d(mat, tmp);
        test5(tmp);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100<< "ms" << endl;
    compare_d(mat, tmp);

    system("pause");
    return 0;
}
