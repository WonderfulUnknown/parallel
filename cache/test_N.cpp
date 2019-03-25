#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

void test_N(int n)
{
    int **a,*b,*result;
    a = new int *[n];
    b = new int [n];
    result = new int [n];
    for (int i = 0; i < n; i++)
        a[i] = new int[n];

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            a[i][j] = i + j;
        b[i] = i;
    }

    long long head, tail, freq; // timers

    QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // similar to CLOCKS_PER_SEC
    QueryPerformanceCounter((LARGE_INTEGER *)&head);   // start time

    for (int i = 0; i < n; i++)
    {
        result[i] = 0.0;
        for (int j = 0; j < n; j++)
            result[i] += a[i][j] * b[i];
    }

    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "N=" << n << " : " << (tail - head) * 1000.0 / freq << "ms" << endl;

    delete []a;
    delete []b;
    delete []result;
}

int test()
{
    cout << "different N: " <<  endl;
    //test_N(1024);
    //test_N(10240);
    //test_N(102400);
    test_N(32*1);
    test_N(32*2);
    test_N(32*4);
    test_N(32*8);
    test_N(32*16);
    test_N(32*32);
    test_N(32*64);
    test_N(32*128);
    test_N(32*256);
    test_N(32*512);
    test_N(32*1024);
    //test_N(32*2048);
    //test_N(32*4096);
}
