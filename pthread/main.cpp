#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

const int N = 8;
float **mat, **temp, **result;
int thread_count = 4;
int k = 0;

// pthread_mutex_t mutex_task;
// pthread_barrier_t barrier;

sem_t	sem_parent;
sem_t	sem_children;

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

//====================begin================
//信号量实现
void *gauss_thread(void *parm)
{
    int thread_id = (int)((size_t)parm);
    int task_length;
    int task_start;
    int task_end;

    while (k <= N)
    {
        task_length = (N - k + 1) / thread_count;
        task_start = k + 1 + thread_id * task_length;
        if (thread_id != thread_count - 1)
            task_end = task_start + task_length;
        else
            task_end = N;

        for (int i = task_start; i < task_end; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }

        sem_post(&sem_parent);
        sem_wait(&sem_children);
        //fprintf(stdout, "Thread %d is finish,k = %d.\n", thread_id, k);
    }

    pthread_exit(NULL);
}

void thread_message()
{
    int thread;
    pthread_t *thread_handles;
    thread_handles = new pthread_t[thread_count];

    sem_init(&sem_parent, 0, 0);
    sem_init(&sem_children, 0, 0);

    for (int i = k + 1; i < N; i++)
        mat[k][i] = mat[k][i] / mat[k][k];
    mat[k][k] = 1;
    k++;

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, *gauss_thread, (void *)thread);

    for (thread = 0; thread < thread_count; thread++)
        sem_wait(&sem_parent); //--

    while (k <= N)
    {
        for (int i = k + 1; i < N; i++)
            mat[k][i] = mat[k][i] / mat[k][k];
        mat[k][k] = 1;

        for (thread = 0; thread < thread_count; thread++)
            sem_post(&sem_children); //++
        for (thread = 0; thread < thread_count; thread++)
            sem_wait(&sem_parent);//--
        
        k++;
    }

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

	sem_destroy(&sem_parent);
	sem_destroy(&sem_children);

    free(thread_handles);
}
//====================end=================

int main()
{
    long long head, tail, freq; // timers
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    mat = new float *[N];
    temp = new float *[N];
    result = new float *[N];
    for (int i = 0; i < N; i++)
    {
        mat[i] = new float[N];
        temp[i] = new float[N];
        result[i] = new float[N];
    }
    srand((unsigned)time(NULL));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            mat[i][j] = rand() % 100;

    cout << "============= N=" << N << "============" << endl;

    copy(mat, temp);
    cout << "普通高斯消去" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy(mat, result);
        normal_gauss(result);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    print(result);

    cout << endl;
    cout << "信号量实现" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    for (int i = 0; i < 100; i++)
    {
        copy(temp, mat);
        thread_message();
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    compare(result, mat);
    print(mat);

    return 0;
}
