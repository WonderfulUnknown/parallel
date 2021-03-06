#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

const int N = 10240;

double a[N][N], b[N], result[N];

void init() // generate a N*N matrix
{
  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
      a[i][j] = i + j;
    b[i] = i;
  }
}

int main()
{
  long long head, tail, freq; // timers
  init();

  QueryPerformanceFrequency((LARGE_INTEGER *)&freq); // similar to CLOCKS_PER_SEC
  QueryPerformanceCounter((LARGE_INTEGER *)&head);   // start time

  //cache优化
  for (int i = 0; i < N; i++)
  {
    result[i] = 0.0;
    for (int j = 0; j < N; j++)
      result[i] += a[i][j] * b[i];
  }

  QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
  cout << "Col: " << (tail - head) * 1000.0 / freq << "ms" << endl;

  //未优化
  for (int j = 0; j < N; j++)
  {
    result[j] = 0.0;
    for (int i = 0; i < N; i++)
      result[i] += a[i][j] * b[i];
  }

  QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
  cout << "Col: " << (tail - head) * 1000.0 / freq << "ms" << endl;
}
