#include <iostream>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"

using namespace std;

const int N = 4096;
float **mat, **temp, **answer;
long long head, freq;

void print(float **mat) //输出
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            cout << mat[i][j] << " ";
        cout << endl;
    }
}

void compare(float **m1, float **m2)
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

void copy(float **m1, float **m2)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            m2[i][j] = m1[i][j];
    }
}

void normal_gauss(float **mat) //普通高斯消去法
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

void test1(int argc, char *argv[])
{
	int counts, my_id;
	float **result = new (float *)[N];
	float *msg = new float [N];
    for (int i = 0; i < N; i++)
        result[i] = new float [N];

	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_id);  //报告识别调用进程的rank
	MPI_Comm_size(MPI_COMM_WORLD, &counts); //报告进程数

	int block_size = N / counts;

	if (my_id == 0)
	{
		gettimeofday(&pcstart, NULL);
		//0号进程把矩阵传递给其他进程
		for (int i = my_id + 1; i < counts; i++)
			for (int j = i * (N / counts); j < (i + 1) * (N / counts); j++)
				MPI_Send(mat[j], N, MPI_FLOAT, i, 99, MPI_COMM_WORLD);
		gettimeofday(&pstart, NULL);
	}
	else
	{
		for (int i = 0; i < N / counts; i++)
			MPI_Recv(temp[i], N, MPI_FLOAT, 0, 99, MPI_COMM_WORLD, &status);
	}

	int m = 0;
	for (int k = 0; k < N; k++)
	{
		if (k < (my_id + 1) * block_size && k >= my_id * block_size)
		{
			for (int j = k + 1; j < N; j++)
				temp[m][j] = temp[m][j] / temp[m][k];
			temp[m][k] = 1;
			for (int j = 0; j < n; j++)
				msg[j] = temp[m][j];
			m++;//保证按序，需要考虑是否有其他更好的写法
		}
		//将计算好的结果广播
		MPI_Bcast(msg, N, MPI_FLOAT, k / block_size, MPI_COMM_WORLD);
		if (k < (my_id + 1) * block_size)
		{
			for (int i = m; i < block_size; i++)
			{
				for (int j = k + 1; j < n; j++)
					result[i][j] = result[i][j] - result[i][k] * msg[j];
				result[i][k] = 0;
			}
		}
	}
	MPI_Finalize();
}

int main(int argc, char *argv[])
{
	long long tail;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

	mat = new float *[N];
	temp = new float *[N];
	answer = new float *[N];

    for (int i = 0; i < N; i++)
    {
        mat[i] = new float[N];
		temp[i] = new float[N];
		answer[i] = new float[N];
	}

	srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 100;

    cout << "============= N=" << N << "============" << endl;

    copy(mat, temp);
    cout << "普通高斯消去" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    copy(mat, answer);
    normal_gauss(answer);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;

    cout << endl;
    cout << "MPI" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    copy(temp, mat);
    test1(&argc, &argv);
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(answer, mat);

	return 0;
}
