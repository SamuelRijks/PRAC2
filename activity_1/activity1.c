#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define SHM_SIZE sizeof(int)

int main(void)
{
    pid_t pid;
    int *shmaddr;
    sem_t *sem1, *sem2;
    int shmfd;

    // Create the shared memory segment
    shmfd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    if (shmfd == -1)
    {
        perror("shm_open");
        exit(1);
    }

    // Set the size of the shared memory to 4 bytes
    if (ftruncate(shmfd, SHM_SIZE) == -1)
    {
        perror("ftruncate");
        exit(1);
    }

    // Map the shared memory segment to the process's address space
    shmaddr = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shmaddr == MAP_FAILED)
    {
        perror("mmap");
        exit(1);
    }

    // Generate a random number between 1 and 10
    srand(time(NULL));
    int random_num = rand() % 10 + 1;
    *shmaddr = random_num;

    // Initialize the two semaphores
    if (sem_init(&sem1, 1, 1) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    if (sem_init(&sem2, 1, 0) == -1)
    {
        perror("sem_init");
        exit(1);
    }

    // Create the two named semaphores
    sem1 = sem_open("/activity1_sem1", O_CREAT, 0666, 1);
    if (sem1 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    sem2 = sem_open("/activity2_sem2", O_CREAT, 0666, 0);
    if (sem2 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    // Fork the process
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(1);
    }

    if (pid == 0)
    {
        // Child process
        while (*shmaddr > 0)
        {
            printf("Child: waiting for sem1...\n");
            sem_wait(sem1);
            if (*shmaddr > 0)
            {
                printf("Child: acquired sem1, decrementing shared memory...\n");
                *shmaddr -= 1;
            }
            printf("Child: releasing sem2...\n");
            sem_post(sem2);
        }
    }
    else
    {
        // Parent process
        while (*shmaddr > 0)
        {
            printf("Parent: waiting for sem2...\n");
            sem_wait(sem2);
            if (*shmaddr > 0)
            {
                printf("Parent: acquired sem2, decrementing shared memory...\n");
                *shmaddr -= 1;
            }
            printf("Parent: releasing sem1...\n");
            sem_post(sem1);
        }
    }

    // Clean up
    munmap(shmaddr, SHM_SIZE);
    close(shmfd);
    shm_unlink("/myshm");
    sem_close(sem1);
    sem_close(sem2);
    sem_unlink("/activity2_sem1");
    sem_unlink("/activity2_sem2");

    return 0;
}
