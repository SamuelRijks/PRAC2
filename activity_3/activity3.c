#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

typedef struct
{
    uint32_t *data_ptr;
    pthread_mutex_t *mutex1;
    pthread_mutex_t *mutex2;
} thread_data_t;

void *thread1_func(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    while (*(data->data_ptr) > 0)
    {
        pthread_mutex_lock(data->mutex1);
        if (*(data->data_ptr) > 0)
        {
            printf("Thread 1: bouncing %d\n", *(data->data_ptr));
            *(data->data_ptr) = *(data->data_ptr) - 1;
            pthread_mutex_unlock(data->mutex2);
        }
        else
        {
            pthread_mutex_unlock(data->mutex1);
        }
    }
    printf("Thread 1: ends\n");
    return NULL;
}

void *thread2_func(void *arg)
{
    thread_data_t *data = (thread_data_t *)arg;
    while (*(data->data_ptr) > 0)
    {
        pthread_mutex_lock(data->mutex2);
        if (*(data->data_ptr) > 0)
        {
            printf("Thread 2: bouncing %d\n", *(data->data_ptr));
            *(data->data_ptr) = *(data->data_ptr) - 1;
            pthread_mutex_unlock(data->mutex1);
        }
        else
        {
            pthread_mutex_unlock(data->mutex2);
        }
    }
    printf("Thread 2: ends\n");
    return NULL;
}

int main()
{
    srand(time(NULL));                     // seed the random number generator with current time
    uint32_t shared_var = rand() % 20 + 1; // generate random number between 1 and 20
    printf("main: bouncing for %d times.\n", shared_var);
    pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

    thread_data_t data = {&shared_var, &mutex1, &mutex2};

    pthread_t thread1, thread2;

    pthread_mutex_lock(&mutex2);

    printf("main: bouncing for %d times.\n", shared_var);

    pthread_create(&thread1, NULL, thread1_func, (void *)&data);
    pthread_create(&thread2, NULL, thread2_func, (void *)&data);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mutex1);
    pthread_mutex_destroy(&mutex2);

    return 0;
}
