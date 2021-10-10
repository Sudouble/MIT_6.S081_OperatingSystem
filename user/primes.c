#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void Prime(int nFD)
{
    int nNum_P = 0;
    int nResult = read(nFD, &nNum_P, sizeof(int));
    if (nResult == 0)
    {
        exit(0);
    }

    fprintf(1, "prime %d\n", nNum_P);
    int pipFD[2] = {0};
    pipe(pipFD);

    if (fork() == 0)
    {
        close(pipFD[1]);
        Prime(pipFD[0]);
        exit(0);
    }
    else
    {
        close(pipFD[0]);

        int nNum_N = 0;
        while(read(nFD, &nNum_N, sizeof(int)))
        {
            if(nNum_N % nNum_P != 0)
            {
                write(pipFD[1], &nNum_N, sizeof(int));
            }
        }
        close(pipFD[1]);
        wait((int*)0);
    }
}


int
main(int argc, char *argv[])
{
    int pipeFD[2] = {0};
    pipe(pipeFD);

    int childPID = fork();
    if (childPID == -1)
    {
        char* pErrMsg = "Fork error";
        write(1, pErrMsg, strlen(pErrMsg));
        write(1, "\n", 1);
    }
    else if (childPID == 0)    // child process
    {
        close(pipeFD[1]);
        Prime(pipeFD[0]);
        exit(0);
    }
    else  // parent process
    {
        close(pipeFD[0]);
        for (int i = 2; i <= 35; ++i)
        {
            write(pipeFD[1], &i, sizeof(int));   
        }
        close(pipeFD[1]);
    }

    wait((int*)0);
    exit(0);
}
