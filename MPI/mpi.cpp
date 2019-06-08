#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include "mpi.h"

using namespace std;

const int N = 1024;
float mat[N][N], temp[N][N], answer[N][N], result[N][N];
int counts, my_id;

void print(float mat[N][N]) //���
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

void normal_gauss() //��ͨ��˹��ȥ��
{
    srand(time(0));
    struct timeval tstart, tend;
    gettimeofday(&tstart, NULL);
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

    gettimeofday(&tend, NULL);
    cout << "��ͨ��˹��ȥ: " << (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000 << " ms" << endl;
}

int main(int argc, char *argv[])
{
    /*MPI_Status status;*/
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);  //����ʶ����ý��̵�rank
    MPI_Comm_size(MPI_COMM_WORLD, &counts); //���������
    if (my_id == 0)
    {
        srand((unsigned)time(NULL));
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                mat[i][j] = rand() % 100;

        cout << "============= N=" << N << "============" << endl;

        copy(mat, temp);
        normal_gauss();
        copy(mat, answer);
        /*print(answer);*/
        copy(temp, mat);
    }

    srand(time(0));
    struct timeval tstart, tend;
    gettimeofday(&tstart, NULL);

    /*�Ѿ���㲥�����н���*/
    MPI_Bcast(mat, N * N, MPI_FLOAT, 0, MPI_COMM_WORLD);
    int block_size = N / counts;
    int begin = my_id * block_size;
    int end = (my_id + 1) * block_size;
    int root = 0;
    for (int k = 0; k < N; k++)
    {
        root = k / block_size;
        if (k >= begin && k < end)
        {
            for (int j = k + 1; j < N; j++)
                mat[k][j] = mat[k][j] / mat[k][k];
            mat[k][k] = 1;
        }
        /*���Լ�����õĽ���㲥*/
        MPI_Bcast(mat[k], N, MPI_FLOAT, root, MPI_COMM_WORLD);

        for (int i = k + 1; i < N; i++)
        {
            root = i / block_size;
            if (i >= begin && i < end)
            {
                for (int j = k + 1; j < N; j++)
                    mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
                mat[i][k] = 0;
            }
            MPI_Bcast(mat[i], N, MPI_FLOAT, root, MPI_COMM_WORLD);
        }
    }

    if (my_id == 0)
    {
        gettimeofday(&tend, NULL);
        cout << endl;
        cout << "MPI�黮��: " << (tend.tv_sec - tstart.tv_sec) * 1000 + (tend.tv_usec - tstart.tv_usec) / 1000 << " ms" << endl;
        compare(answer, mat);
        /*print(mat);*/
    }

    MPI_Finalize();
    return 0;
}
