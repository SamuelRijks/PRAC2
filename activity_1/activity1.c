#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <semaphore.h>

#define SHM_SIZE 1024

int main(void)
{
    pid_t pid;
    key_t key;
    int shmid;
    char *shmaddr;
    sem_t *sem1, *sem2;

    // Generate a key for the shared memory segment
    key = ftok(".", 'a');
    if (key == -1)
    {
        perror("ftok");
        exit(1);
    }

    // Create the shared memory segment
    shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    // Attach the shared memory segment to the process's address space
    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (char *)-1)
    {
        perror("shmat");
        exit(1);
    }

    // Initialize the shared memory to "hello"
    sprintf(shmaddr, "hello");

    // Create the two named semaphores
    sem1 = sem_open("/sem1", O_CREAT, 0666, 1);
    if (sem1 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    sem2 = sem_open("/sem2", O_CREAT, 0666, 0);
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
        printf("Child: waiting for sem1...\n");
        sem_wait(sem1);
        printf("Child: acquired sem1, reading shared memory...\n");
        printf("Child: shared memory contains: %s\n", shmaddr);
        printf("Child: writing 'world' to shared memory...\n");
        sprintf(shmaddr, "world");
        printf("Child: releasing sem2...\n");
        sem_post(sem2);
    }
    else
    {
        // Parent process
        printf("Parent: waiting for sem2...\n");
        sem_wait(sem2);
        printf("Parent: acquired sem2, reading shared memory...\n");
        printf("Parent: shared memory contains: %s\n", shmaddr);
        printf("Parent: releasing sem1...\n");
        sem_post(sem1);
    }

    // Clean up
    shmdt(shmaddr);
    shmctl(shmid, IPC_RMID, NULL);
    sem_close(sem1);
    sem_close(sem2);
    sem_unlink("/sem1");
    sem_unlink("/sem2");

    return 0;
}
