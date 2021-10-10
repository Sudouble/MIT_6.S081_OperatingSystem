#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc == 1)
  {
      char* pErrorMsg = "no paramter, usage: sleep number";
      write(1, pErrorMsg, strlen(pErrorMsg));
      write(1, "\n", 1);
  }
  else if (argc > 2)
  {
      char* pErrorMsg = "paramter too many!";
      write(1, pErrorMsg, strlen(pErrorMsg));
      write(1, "\n", 1);
  }
  else
  {
      int nSecond = atoi(argv[1]);
      sleep(nSecond);
      write(1, "\n", 1);
  }

  exit(0);
}
