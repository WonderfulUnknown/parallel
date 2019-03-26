#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

//默认N是2的次幂
const int N = 1024 * 2048;

double a[N], sum;
double temp[N / 2];

void init()
{
    for (int i = 0; i < N; i++)
        a[i] = i;
}

//递归，函数调用影响速度，过慢
// int add(int n)
// {
//     if (n <= 1)
//         return 0;
//     if (n == N) //init
//     {
//         cout<<"n==N"<<endl;
//         for (int i = 0; i < n / 2; i++)
//             temp[i] = a[i] + a[n - i];

//     }
//     for (int i = 0; i < n / 2; i++)
//         temp[i] = temp[i] + temp[n - i];
//     add(n / 2);
//     return temp[0];
// }

int main()
{
    long long head, tail, freq; // timers
    int n;
    init();

    QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // similar to CLOCKS_PER_SEC

    //串行逐个累加
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < N; i++)
        sum += a[i];

    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "Accumulate: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cout << sum << endl;

    //规约算法

    // //单流水
    // n = N;
    // QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    // // int b = add(N);
    // while (n > 1)
    // {
    //     // if (n & 1)
    //     //     a[0] += a[n / 2];
    //     for (int i = 0; i < n / 2; i++)
    //         a[i] += a[n - i - 1];
    //     n = n / 2;
    // }
    // QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    // cout << "Single: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // cout << a[0] << endl;

    // //双流水
    // n = N;
    // init();
    // QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    // while (n > 1)
    // {
    //     // if (n & 1)
    //     //     a[0] += a[n / 2];
    //     for (int i = 0; i < n / 2; i += 2)
    //     {
    //         a[i] += a[n - i - 1];
    //         a[i + 1] += a[n - i - 2];
    //     }
    //     n = n / 2;
    // }
    // QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    // cout << "Double: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // cout << a[0] << endl;

    // //四条流水
    // n = N;
    // init();
    // QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    // while (n > 1)
    // {
    //     // if (n & 1)
    //     //     a[0] += a[n / 2];
    //     for (int i = 0; i < n / 2; i += 4)
    //     {
    //         a[i] += a[n - i - 1];
    //         a[i + 1] += a[n - i - 2];
    //         a[i + 2] += a[n - i - 3];
    //         a[i + 3] += a[n - i - 4];
    //     }
    //     n = n / 2;
    // }
    // QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    // cout << "Quadruple: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // cout << a[0] << endl;

    // //八条流水
    // n = N;
    // init();
    // QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    // while (n > 1)
    // {
    //     // if (n & 1)
    //     //     a[0] += a[n / 2];
    //     for (int i = 0; i < n / 2; i += 8)
    //     {
    //         a[i] += a[n - i - 1];
    //         a[i + 1] += a[n - i - 2];
    //         a[i + 2] += a[n - i - 3];
    //         a[i + 3] += a[n - i - 4];
    //         a[i + 4] += a[n - i - 5];
    //         a[i + 5] += a[n - i - 6];
    //         a[i + 6] += a[n - i - 7];
    //         a[i + 7] += a[n - i - 8];
    //     }
    //     n = n / 2;
    // }
    // QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    // cout << "Octuple: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // cout << a[0] << endl;

    n = N;
    init();
    sum = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < n; i += 4)
    {
        temp[0] = a[i] + a[i + 1];
        temp[1] = a[i + 2] + a[i + 3];
        sum += temp[0] + temp[1];
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "Dobule: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cout << sum << endl;

    sum = 0;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < n; i += 8)
    {
        temp[0] = a[i] + a[i + 1];
        temp[1] = a[i + 2] + a[i + 3];
        temp[2] = a[i + 4] + a[i + 5];
        temp[3] = a[i + 6] + a[i + 7];
        sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "Quadruple: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cout << sum << endl;
}
