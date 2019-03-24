#include <iostream>
#include <windows.h>
#include <stdlib.h>

using namespace std;

const int N = 10240;		// matrix size

double b[N][N], col_sum[N];

void init(int n)			// generate a N*N matrix
{
  for (int i = 0; i < N; i++)
    for (int j = 0; j < N; j++)
      b[i][j] = i + j;
}
int main1()
{
     long long head, tail, freq;        // timers

	init(N);

	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);	// similar to CLOCKS_PER_SEC
 	QueryPerformanceCounter((LARGE_INTEGER *)&head);	// start time
     for (int i = 0; i < 1000; i++) {
       col_sum[i] = 0.0;
       for (int j = 0; j < 1000; j++)
       col_sum[i] += b[j][i];
     }
	QueryPerformanceCounter((LARGE_INTEGER *)&tail);	// end time
	cout << "Col: " << (tail - head) * 1000.0 / freq << "ms" << endl;


	QueryPerformanceCounter((LARGE_INTEGER *)&head);	// start time
	    for (int i = 0; i < 1000; i++) {
       col_sum[i] = 0.0;
       for (int j = 0; j < 1000; j++)
       col_sum[j] += b[i][j];
     }
	QueryPerformanceCounter((LARGE_INTEGER *)&tail);	// end time
	cout << "Col: " << (tail - head) * 1000.0 / freq << "ms" << endl;
}
