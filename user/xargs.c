#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[])
{
  char inputBuf[32]; // record the input from previous command
  char charBuf[320]; // buf for the char of all token
  char* charBufPointer = charBuf;
  int charBufSize = 0;

  char *commandToken[32];
  int tokenSize = argc - 1;
  int inputSize = -1;

  // first copy initial argv argument to commandToken
  for(int tokenIdx=0;tokenIdx<tokenSize;tokenIdx++)
    commandToken[tokenIdx]=argv[tokenIdx+1];

  while((inputSize = read(0,inputBuf,sizeof(inputBuf))) > 0)
  {
    for(int i = 0; i < inputSize;i++)
    {
      char curChar = inputBuf[i];
      if(curChar == '\n')
      {
        // if read '\n', execute the command
        charBuf[charBufSize] = 0; // set '\0' to end of token
        commandToken[tokenSize++] = charBufPointer;
        commandToken[tokenSize] = 0; // set nullptr in the end of array

        if(fork()==0)
        {
        //   fprintf(1, "executing: %s\n, token: ", argv[1]);
        //   for (int ii = 0; ii < tokenSize; ++ii)
        //   {
        //       fprintf(1, "%s ", commandToken[ii]);
        //   }
        //   fprintf(1, "\n");
          // open child to execute
          exec(argv[1],commandToken);
        }
        wait(0);
        tokenSize = argc - 1;// initialize
        charBufSize = 0;
        charBufPointer = charBuf;
      }
      else if(curChar == ' ')
      {
        // split by ' '
        charBuf[charBufSize++] = 0; // mark the end of string
        commandToken[tokenSize++] = charBufPointer;
        charBufPointer = charBuf+charBufSize;
      }else
      {
        charBuf[charBufSize++] = curChar;
      }

    }
  }
  exit(0);
}