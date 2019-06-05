#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
int n = 4096;
int main(int argc, char *argv[])
{
  int counts, rank, namelen;
  MPI_Status status;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Get_processor_name(processor_name, &namelen);
  MPI_Comm_size(MPI_COMM_WORLD, &counts);

  int block_size = n / counts;
  float result;
  double susetime;
  double pusetime;
  double pusetime_cost;
  timeval pstart, pend, pcstart, pcend;
  float **b;
  float **a = (float **)malloc((n / counts) * sizeof(float *));
  float *aa = (float *)malloc(n * sizeof(float));

  for (int i = 0; i < n / counts; i++)
    a[i] = (float *)malloc(n * sizeof(float));
  if (rank == 0)
  {
    srand((unsigned)time(NULL));
    b = (float **)malloc(n * sizeof(float *));

    for (int i = 0; i < n; i++)
      b[i] = (float *)malloc(n * sizeof(float));
    float **c = (float **)malloc(n * sizeof(float *));
    for (int i = 0; i < n; i++)
      c[i] = (float *)malloc(n * sizeof(float));

    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
        c[i][j] = b[i][j] = rand() % 100 + 1;

    timeval start, end;
    gettimeofday(&start, NULL);
    for (int k = 0; k < n; k++)
    {
      for (int i = k + 1; i < n; i++)
        c[k][i] = c[k][i] / c[k][k];
      c[k][k] = 1.0f;
      for (int i = k + 1; i < n; i++)
        for (int j = k + 1; j < n; j++)
          c[i][j] = c[i][j] - c[i][k] * c[k][j];
      c[i][k] = 0.0f;
    }
    gettimeofday(&end, NULL);
    susetime = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    susetime /= 1000;
    cout << "susetime:" << susetime << endl;
    result = c[n - 2][n - 1];

    for (int i = 0; i < n; i++)
      free(c[i]);
    free(c);

    for (int i = 0; i < n / counts; i++)
      for (int j = 0; j < n; j++)
        a[i][j] = b[i][j];

    gettimeofday(&pcstart, NULL);
    for (int i = rank + 1; i < counts; i++)
      for (int j = i * (n / counts); j < (i + 1) * (n / counts); j++)
        MPI_Send(b[j], n, MPI_FLOAT, i, 99, MPI_COMM_WORLD);
    gettimeofday(&pstart, NULL);
  }
  else
  {
    for (int i = 0; i < n / counts; i++)
      MPI_Recv(a[i], n, MPI_FLOAT, 0, 99, MPI_COMM_WORLD, &status);
  }

  int m = 0;
  for (int k = 0; k < n; k++)
  {
    int root;
    root = k / block_size;
    if (k < (rank + 1) * block_size && k >= rank * block_size)
    {
      for (int j = k + 1; j < n; j++)
        a[m][j] = a[m][j] / a[m][k];
      a[m][k] = 1.0f;
      for (int j = 0; j < n; j++)
        aa[j] = a[m][j];
      m = m + 1;
    }

    MPI_Bcast(aa, n, MPI_FLOAT, root, MPI_COMM_WORLD);
    if (rank == 0)
    {
      for (int j = 0; j < n; j++)
        b[k][j] = aa[j];
    }
    if (k < (rank + 1) * block_size)
    {
      for (int i = m; i < block_size; i++)
      {
        for (int j = k + 1; j < n; j++)
          a[i][j] = a[i][j] - a[i][k] * aa[j];
        a[i][k] = 0.0f;
      }
    }
  }
  if (rank == 0)
  {
    gettimeofday(&pend, NULL);
    gettimeofday(&pcend, NULL);
    pusetime = 1000000 * (pend.tv_sec - pstart.tv_sec) + pend.tv_usec - pstart.tv_usec;
    pusetime /= 1000;

    cout << "pusetime:" << pusetime << endl;
    cout << "pusetime:Speedup " << susetime / pusetime << endl;
    cout << "pusetime:E" << susetime / (pusetime * counts) << endl;

    pusetime_cost = 1000000 * (pcend.tv_sec - pcstart.tv_sec) + pcend.tv_usec - pcstart.tv_usec;
    pusetime_cost /= 1000;

    cout << "pusetime_cost" << pusetime_cost << endl;
    cout << "pusetime_cost:Speedup" << susetime / pusetime_cost << endl;
    cout << "pusetime_cost:E" << susetime / (pusetime_cost * counts) << endl;

    if (result == b[n - 2][n - 1])
      cout << "success!" << endl;
    else
      cout << "false!" << endl;
    for (int i = 0; i < n; i++)
      free(b[i]);
    free(b);
  }
  MPI_Finalize();
  return 0;
}
