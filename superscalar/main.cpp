#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

const int N = 10240;

double a[N], result;
//double temp[N / 2];

void init()
{
    for (int i = 0; i < N; i++)
        a[N] = i;
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
    init();

    QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // similar to CLOCKS_PER_SEC

    //串行逐个累加
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < N; i++)
        result += a[i];

    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "串行逐个累加: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cout << result << endl;

    //规约算法
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    // int b = add(N);
    int n = N;
    while(n>1)
    {
        for(int i =0;i<n/2;i++)
            a[i] = a[i] + a[n - i];
        n = n/2;
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "规约算法: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    cout << a[0] << endl;
}
