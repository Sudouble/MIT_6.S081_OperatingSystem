#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pipeFD[2] = {0};
  char buff = 0;

  pipe(pipeFD);
  fprintf(1, "process\b");
  
  int childPID = fork();
  if (childPID == -1)
  {
      char* pErrMsg = "Fork error";
      write(1, pErrMsg, strlen(pErrMsg));
      write(1, "\n", 1);
  }
  else if (childPID == 0)    // child process
  {
      int npid = getpid();
      read(pipeFD[0], &buff, sizeof(char));
      if (buff == '2')
      {
          fprintf(1, "%d: received ping\n", npid);
      }
      write(pipeFD[1], "1", 1);
  }
  else  // parent process
  {
      int npid = getpid();
      write(pipeFD[1], "2", 1);
      wait((int*)0);

      read(pipeFD[0], &buff, sizeof(char));
      if (buff == '1')
      {
          fprintf(1, "%d: received pong\n", npid);
      }
  }

  exit(0);
}
