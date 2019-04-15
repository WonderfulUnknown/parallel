#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>  

const int N = 8;

using namespace std;

void normal_gaosi(float matrix[N][N]) //��ͨ��˹��ȥ��
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

void SSE_gaosi(float matrix[N][N]) //SSE���л�
{
    __m128 t1, t2, t3, t4;
    for (int k = 0; k < N; k++)
    {
        float temp[4] = {matrix[k][k], matrix[k][k], matrix[k][k], matrix[k][k]};
        t1 = _mm_loadu_ps(temp);
        for (int j = N - 4; j >= k; j -= 4)
        {
            t2 = _mm_loadu_ps(matrix[k] + j);
            t3 = _mm_div_ps(t2, t1); //����
            _mm_storeu_ps(matrix[k] + j, t3);
        }
        // matrix[k][k] = 1.0;
        if (k % 4 != (N % 4)) //�����ܱ�4������Ԫ��
        {
            for (int j = k; j % 4 != (N % 4); j++)
                matrix[k][j] = matrix[k][j] / temp[0];
        }
        for (int j = (N % 4) - 1; j >= 0; j--)
            matrix[k][j] = matrix[k][j] / temp[0];
        for (int i = k + 1; i < N; i++)
        {
            float temp[4] = {matrix[i][k], matrix[i][k], matrix[i][k], matrix[i][k]};
            t1 = _mm_loadu_ps(temp);
            for (int j = N - 4; j > k; j -= 4)
            {
                t2 = _mm_loadu_ps(matrix[i] + j);
                t3 = _mm_loadu_ps(matrix[k] + j);
                t4 = _mm_sub_ps(t2, _mm_mul_ps(t1, t3)); //����
                _mm_storeu_ps(matrix[i] + j, t4);
            }
            for (int j = k + 1; j % 4 != (N % 4); j++)
                matrix[i][j] = matrix[i][j] - matrix[i][k] * matrix[k][j];
            matrix[i][k] = 0;
        }
    }
}

void print(float matrix[N][N]) //���
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            cout << matrix[i][j] << " ";
        cout << endl;
    }
}

int main()
{
    long long head, tail, freq; // timers
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    float matrix[N][N],copy[N][N];
    srand((unsigned)time(NULL));
    cout << "�����������" << endl;
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
    cout << "��ͨ��˹��ȥ��" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);   // start time
    normal_gaosi(matrix);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "��ʱ�� " << (tail - head) * 1000.0 / freq  << "ms" << endl;
    print(matrix);

    cout << endl;
    cout << "ʹ��SSE����" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head);   // start time
    SSE_gaosi(copy);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "��ʱ�� " << (tail - head) * 1000.0 / freq  << "ms" << endl;
    print(copy);

    system("pause");
    return 0;
}
