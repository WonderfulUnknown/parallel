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
long long head, freq;

pthread_mutex_t mutex;
pthread_barrier_t barrier;

sem_t sem_parent;
sem_t sem_children;

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

//===================begin================
//信号量实现 + 对循环划分
void *pthread1(void *parm)
{
    int thread_id = (int)((size_t)parm);
    int task_length;
    int task_start;
    int task_end;

    while (N - k > thread_count)
    {
        task_length = (N - k - 1) / thread_count;
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
    }
    pthread_exit(NULL);
}

void test1()
{
    int thread;
    pthread_t *thread_handles;
    thread_handles = new pthread_t[thread_count];

    sem_init(&sem_parent, 0, 0);
    sem_init(&sem_children, 0, 0);

    for (int i = k + 1; i < N; i++)
        mat[k][i] = mat[k][i] / mat[k][k];
    mat[k][k] = 1;

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, *pthread1, (void *)thread);

    for (thread = 0; thread < thread_count; thread++)
        sem_wait(&sem_parent); //--

    while (1)
    {
        k++;
        if (k == N - thread_count)
        {
            for (thread = 0; thread < thread_count; thread++)
                sem_post(&sem_children); //++
            break;
        }
        for (int i = k + 1; i < N; i++)
            mat[k][i] = mat[k][i] / mat[k][k];
        mat[k][k] = 1;
        for (thread = 0; thread < thread_count; thread++)
            sem_post(&sem_children); //++
        for (thread = 0; thread < thread_count; thread++)
            sem_wait(&sem_parent); //--
    }

    for (k; k < N; k++)
    {
        for (int i = k + 1; i < N; i++)
            mat[k][i] = mat[k][i] / mat[k][k];
        mat[k][k] = 1;
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    sem_destroy(&sem_parent);
    sem_destroy(&sem_children);

    free(thread_handles);
}
//====================end=================

//===================begin================
//信号量实现 + SSE + 对循环划分
void *pthread2(void *parm)
{
    int thread_id = (int)((size_t)parm);
    int task_length;
    int task_start;
    int task_end;
    __m128 t1, t2, t3;
    float a[4];
    while (N - k > thread_count)
    {
        task_length = (N - k - 1) / thread_count;
        task_start = k + 1 + thread_id * task_length;
        if (thread_id != thread_count - 1)
            task_end = task_start + task_length;
        else
            task_end = N;

        for (int i = task_start; i < task_end; i++)
        {
            for (int x = 0; x < 4; x++)
                a[x] = mat[i][k];
            t1 = _mm_loadu_ps(a);
            for (int j = N - 4; j >= k + 1; j -= 4)
            {
                t2 = _mm_loadu_ps(mat[i] + j);
                t3 = _mm_loadu_ps(mat[k] + j);
                t3 = _mm_mul_ps(t1, t3);
                t3 = _mm_sub_ps(t2, t3);
                _mm_storeu_ps(mat[i] + j, t3);
            }
            for (int j = k + 1; j % 4; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }

        sem_post(&sem_parent);
        sem_wait(&sem_children);
    }
    pthread_exit(NULL);
}

void test2()
{
    int thread;
    pthread_t *thread_handles;
    thread_handles = new pthread_t[thread_count];

    sem_init(&sem_parent, 0, 0);
    sem_init(&sem_children, 0, 0);

    for (int i = k + 1; i < N; i++)
        mat[k][i] = mat[k][i] / mat[k][k];
    mat[k][k] = 1;

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, *pthread2, (void *)thread);

    for (thread = 0; thread < thread_count; thread++)
        sem_wait(&sem_parent); //--

    while (1)
    {
        k++;
        if (k == N - thread_count)
        {
            for (thread = 0; thread < thread_count; thread++)
                sem_post(&sem_children); //++
            break;
        }
        for (int i = k + 1; i < N; i++)
            mat[k][i] = mat[k][i] / mat[k][k];
        mat[k][k] = 1;
        for (thread = 0; thread < thread_count; thread++)
            sem_post(&sem_children); //++
        for (thread = 0; thread < thread_count; thread++)
            sem_wait(&sem_parent); //--
    }

    for (k; k < N; k++)
    {
        for (int i = k + 1; i < N; i++)
            mat[k][i] = mat[k][i] / mat[k][k];
        mat[k][k] = 1;
        for (int i = k + 1; i < N; i++)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
    }

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    sem_destroy(&sem_parent);
    sem_destroy(&sem_children);

    free(thread_handles);
}
//====================end=================

//===================begin================
//对矩阵进行划分 使用barrier
void *pthread3(void *parm)
{
    int thread_id = (int)((size_t)parm);

    //long long tail;

    for (int k = 0; k < N; k++)
    {
        if ((k % thread_count) == thread_id)
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
        pthread_barrier_wait(&barrier);
    }

    // pthread_mutex_lock(&mutex);
    // QueryPerformanceCounter((LARGE_INTEGER *)&tail);
    // cout << "Thread " << thread_id << ": " << (tail - head) * 1000.0 / freq << "ms" << endl;
    // pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

void test3()
{
    int thread;
    pthread_t *thread_handles;
    thread_handles = new pthread_t[thread_count];

    pthread_barrier_init(&barrier, NULL, thread_count);

    // QueryPerformanceCounter((LARGE_INTEGER*)&head); // start timer
    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, *pthread3, (void *)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    pthread_barrier_destroy(&barrier);
    free(thread_handles);
}
//====================end=================

//===================begin================
//对循环进行划分 使用barrier
void *pthread4(void *parm)
{
    int thread_id = (int)((size_t)parm);
    for (int k = 0; k < N; k++)
    {
        for (int j = k + 1; j < N; j++)
            mat[k][j] = mat[k][j] / mat[k][k];
        mat[k][k] = 1.0;

        for (int i = k + 1 + thread_id; i < N; i += thread_count)
        {
            for (int j = k + 1; j < N; j++)
                mat[i][j] = mat[i][j] - mat[i][k] * mat[k][j];
            mat[i][k] = 0;
        }
        pthread_barrier_wait(&barrier);
    }
    pthread_exit(NULL);
}

void test4()
{
    int thread;
    pthread_t *thread_handles;
    thread_handles = new pthread_t[thread_count];

    pthread_barrier_init(&barrier, NULL, thread_count);

    for (thread = 0; thread < thread_count; thread++)
        pthread_create(&thread_handles[thread], NULL, *pthread4, (void *)thread);

    for (thread = 0; thread < thread_count; thread++)
        pthread_join(thread_handles[thread], NULL);

    pthread_barrier_destroy(&barrier);
    free(thread_handles);
}
//====================end=================

int main()
{
    long long tail;
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
    for (int i = 0; i < 10; i++)
    {
        copy(mat, result);
        normal_gauss(result);
    }
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq / 10 << "ms" << endl;
    //print(result);

    cout << endl;
    cout << "循环划分+信号量实现" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    //for (int i = 0; i < 100; i++)
    //{
    copy(temp, mat);
    test1();
    //}
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    //cout << "耗时：" << (tail - head) * 1000.0 / freq / 100 << "ms" << endl;
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    //print(mat);

    k = 0;
    cout << endl;
    cout << "循环划分+信号量+SSE实现" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    copy(temp, mat);
    test2();
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    //print(mat);

    k = 0;
    cout << endl;
    cout << "矩阵划分+barrier实现" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    copy(temp, mat);
    test3();
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    print(mat);

    k = 0;
    cout << endl;
    cout << "循环划分+barrier实现" << endl;
    QueryPerformanceCounter((LARGE_INTEGER *)&head); // start time
    copy(temp, mat);
    test4();
    QueryPerformanceCounter((LARGE_INTEGER *)&tail); // end time
    cout << "耗时：" << (tail - head) * 1000.0 / freq << "ms" << endl;
    compare(result, mat);
    print(mat);

    return 0;
}
