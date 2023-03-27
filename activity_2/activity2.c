#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>

#define MAX_BUF 1024

int main()
{
    int fd;
    pid_t pid;
    char *myfifo = "/tmp/myfifo";
    char buf[MAX_BUF];
    uint8_t a, b;
    uint8_t op_code;
    char op;
    int result;

    /* create the named pipe */
    mkfifo(myfifo, 0666);

    /* fork a child process */
    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "fork failed\n");
        exit(1);
    }
    else if (pid == 0)
    { /* child process */
        printf("Child process started\n");

        /* open the pipe for reading */
        fd = open(myfifo, O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "open failed\n");
            exit(1);
        }

        /* read the first operand from the pipe */
        read(fd, buf, 1);
        a = buf[0];

        /* read the operation from the pipe */
        read(fd, buf, 1);
        op = buf[0];

        /* read the second operand from the pipe */
        read(fd, buf, 1);
        b = buf[0];

        /* perform the mathematical operation */
        switch (op)
        {
        case '+':
            result = a + b;
            break;
        case '-':
            result = a - b;
            break;
        case '*':
            result = a * b;
            break;
        case '/':
            result = a / b;
            break;
        }

        /* close the pipe */
        close(fd);

        printf("The result of %d %c %d is %d\n", a, op, b, result);

        printf("Child process finished\n");
    }
    else
    { /* parent process */
        printf("Parent process started\n");

        /* generate random operands and mathematical operation */
        srand(time(NULL));
        uint8_t a = rand() % 101; /* first operand */
        uint8_t b = rand() % 101; /* second operand */
        char op;                  /* mathematical operation */
        uint8_t op_code;          /* operation code for printing */
        int result;               /* result of the formula */

        /* determine the mathematical operation */
        switch (rand() % 4)
        {
        case 0: /* addition */
            op = '+';
            op_code = 0;
            result = a + b;
            break;
        case 1: /* subtraction */
            op = '-';
            op_code = 1;
            result = a - b;
            break;
        case 2: /* multiplication */
            op = '*';
            op_code = 2;
            result = a * b;
            break;
        case 3: /* division */
            op = '/';
            op_code = 3;
            result = a / b;
            break;
        }

        /* open the pipe for writing */
        fd = open(myfifo, O_WRONLY);
        if (fd < 0)
        {
            fprintf(stderr, "open failed\n");
            exit(1);
        }
        /* write the operands and operation to the pipe */
        write(fd, &a, 1);
        write(fd, &b, 1);
        write(fd, &op_code, 1);
        printf("Sent message to child process: %u %u %c\n", a, b, op);
        /* close the pipe */
        close(fd);

        printf("Parent process finished\n");
        /* remove the named pipe */
        unlink(myfifo);
        exit(0);
    }
}
