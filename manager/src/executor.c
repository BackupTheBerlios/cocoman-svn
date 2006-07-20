#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>




int status = 0;




volatile char inputFileName[1025] = {0};

char * stdoutLog;
int stdoutLogLen = 0;

volatile int isJava = 0;
volatile char classname[2049] = {0};


extern int errno;          /* for reporting additional error information */

struct timeval startTime;
double timeoutPeriod = 10.0;
//double lastJavaExceptionTime = -10000.0;

struct childInfo
{
  char * processName;

  int fdStdout;
  int fdStderr;
  int pid;
  int running;

  int terminatedBySignal; 

  //if terminatedBySignal is 0, exitNum is the return code
  //if terminatedBySignal is 1, exitNum is the signal number
  int exitNum;
};


volatile struct childInfo children[1];
volatile int childIsAlive = 0;

int selfpipe[2];

#define DEAD_QUEUE_SIZE 4
volatile int deadQueue[DEAD_QUEUE_SIZE] = {0};
volatile int deadQueueStartPos = 0;
volatile int deadQueueEndPos = 0;





double getCurrentTime()
{
  struct timeval curTime;
  gettimeofday(&curTime, NULL);

  curTime.tv_sec -= startTime.tv_sec;
  curTime.tv_usec -= startTime.tv_usec;

  if (curTime.tv_usec < 0)
  {
    curTime.tv_sec += 1;
    curTime.tv_usec -= 1000000;
  }

  return (double)curTime.tv_sec + ((double)(curTime.tv_usec) / 1000000.0);
}





void spawnChild(const char * filename, int index)
{
      if (children[index].running != 0)
      {
        printf("Error: Child already running\n"); 
        return;
      }

      //Set up two pipes for IPC
      int fd1[2];
      int fd2[2];
      int p2c[2];

      if (pipe(fd1) < 0)
      {
        printf("Error: pipe() failed\n");
        exit(255);
      }

      if (pipe(fd2) < 0)
      {
        printf("Error: pipe() failed\n");
        exit(255);
      }

      if (pipe(p2c) < 0)
      {
        printf("Error: pipe() failed\n");
        exit(255);
      }
      

      //Fork to create a child process
      long pid = fork();

      //If fork failed, report an error and exit
      if (pid < 0)
      {
        printf ("Error: Could not fork.\n");
        exit(255);
      }
      else if (pid > 0)
      {
        //Parent's code section
        //---------------------

        children[index].processName = (char*)filename;
        children[index].fdStdout = fd1[0];
        children[index].fdStderr = fd2[0];
        children[index].pid = pid;
        children[index].terminatedBySignal = 0;
        children[index].exitNum = 0;
        children[index].running = 1;

        //Close read side of p2c pipe
        close(p2c[0]);

        //Write to p2c pipe informing child that it 
        //can start, then close the p2c pipe
        write(p2c[1], "!", 1);
        close(p2c[1]);

        //Close write side of the pipes
        close(fd1[1]);
        close(fd2[1]);

        return;
      }
      else
      {
        //Child's code section
        //--------------------

        //Use the default signal handler for SIGCHLD
        signal(SIGCHLD, SIG_DFL);

        //Close write side of p2c pipe
        close(p2c[1]);

        //Wait for parent to write to the p2c pipe,
        //then close it
        char tmpbuf[17];
        read(p2c[0], tmpbuf, 16);
        close(p2c[0]);
        
        //Close both ends of the self pipe
        close(selfpipe[0]);
        close(selfpipe[1]);

        //Close read side of the pipes
        close(fd1[0]);
        close(fd2[0]);

        //Open input file and redirect STDIN
        int fdStdin = open((char*)inputFileName, O_RDONLY);
        if (dup2(fdStdin,0) != 0)
          printf("Grrr!!\n");
        close(fdStdin);

        //Redirect STDOUT
        dup2(fd1[1],1);
        close(fd1[1]);

        //Redirect STDERR
        dup2(fd2[1],2);
        close(fd2[1]);

        //Take code from specified executable file
        //and overlay it over this child process
                 
        if (isJava == 0)
        {    
          execlp("soapbox", "soapbox", "-l", "soapbox.log", "-f", 
                 filename, NULL);
          printf ("Error: Could not overlay %s\n", filename);
        }
        else
        {
          execlp("java", "java", "-Djava.security.manager",
                 "-Djava.security.policy=blankpolicyfile",
                 classname, NULL);
          printf ("Error: Could not overlay %s\n", filename);
        }

        exit(255);
      }	
}










void sigchld_handler (int signo)
{
  signal(SIGCHLD, sigchld_handler);

  int old_errno = errno;
  int at_least_one = 0;

  while (1) {
    register int pid;
    int rv;

    /* Keep asking for a status until we get a definitive result.  */
    do
    {
      errno = 0;
      pid = waitpid (-1, &rv, WNOHANG);
    }
    while (pid <= 0 && errno == EINTR);

    if (pid <= 0) {

      /* A real failure means there are no more
         stopped or terminated child processes, so return.  */

      errno = old_errno;

      if (at_least_one != 0)
        write(selfpipe[1], "!", 1);

      return;
    }


    int i = 0;
    
    if (children[0].running == 1 && pid == children[0].pid)
    {
      at_least_one = 1;
      children[0].running = 0;
      deadQueue[deadQueueEndPos] = 0;
      deadQueueEndPos = (deadQueueEndPos+1) % DEAD_QUEUE_SIZE;

      if (WIFSIGNALED(rv))
      {
        children[0].terminatedBySignal = 1;
        children[0].exitNum = WTERMSIG(rv);
      }
      else
      {
        children[0].terminatedBySignal = 0;
        children[0].exitNum = WEXITSTATUS(rv);
      }
    }
    else
    {
      printf("Error: Could not find child in table\n");
      exit(255);
    }
  }
}




int pollRetryOnSignal(struct pollfd fds[], int numfds, int timeout)
{
            int rc = 0;
            int retries = 0;

            if (getCurrentTime() > timeoutPeriod)
            {
              printf("Timed out\n");
              kill(children[0].pid,SIGINT);
              exit(2);
            }

            while ((rc = poll(fds,numfds,timeout)) == -1)
            {
              if (getCurrentTime() > timeoutPeriod)
              {
                printf("Timed out\n");
                kill(children[0].pid,9);
                exit(2);
              }

              if (errno != EINTR && errno != EAGAIN)
                return rc;

              ++retries;
              if (retries > 10)
                return rc;                
            }

            return rc;
}



char buf[257];
int events = (POLLIN | POLLPRI);
int errorEvents = (POLLERR | POLLHUP | POLLNVAL);






void reportStdout(int index)
{
          int n = read(children[index].fdStdout, buf, 256);
          if (n > 0)
          {
            if (stdoutLogLen < 1999000)
            {
              memcpy(stdoutLog+stdoutLogLen,buf,n);
              stdoutLogLen += n;
              stdoutLog[stdoutLogLen] = '\0';
            }

            struct pollfd fds[1];
            fds[0].fd = children[index].fdStdout;
            fds[0].events = events;
            while (pollRetryOnSignal(fds,1,0) >= 0)
            {
              if ((fds[0].revents & events) != 0)
              {
                int n = read(children[index].fdStdout, buf, 256); 
                if (n <= 0) break;

                if (stdoutLogLen < 1999000)
                {
                  memcpy(stdoutLog+stdoutLogLen,buf,n);
                  stdoutLogLen += n;
                  stdoutLog[stdoutLogLen] = '\0';
                }
              }
              else
                break;
            }
          }
}











void reportStderr(int index)
{
          int n = read(children[index].fdStderr, buf, 256);
          if (n > 0)
          {
            buf[n] = '\0';
  
            //char * res = strstr(buf,"Exception ");
            //if (res == NULL)
              //lastJavaExceptionTime = getCurrentTime(); 

            struct pollfd fds[1];
            fds[0].fd = children[index].fdStderr;
            fds[0].events = events;
            while (pollRetryOnSignal(fds,1,0) >= 0)
            {
              if ((fds[0].revents & events) != 0)
              {
                int n = read(children[index].fdStderr, buf, 256); 
                if (n <= 0) break;
                buf[n] = '\0';
              }
              else
                break;
            }
          }
}









int main (int argc, char ** argv)
{
  if (argc != 5)
  {
    printf("usage: executor [executable file] [input file] [output file] [timeout period]\n");
    return 100;
  }


  //Print STDOUT output to the specified file 
  FILE * blankPolicyFile = fopen("blankpolicyfile","w");  
  fwrite("\n",1,1,blankPolicyFile);  
  fclose(blankPolicyFile); 



  pipe(selfpipe);

  signal(SIGCHLD, sigchld_handler);

  childIsAlive = 1;
  gettimeofday(&startTime, NULL);
  int i = 0;
  char execname[1025] = {0};



  int missingDotSlash = 0;
  if (strlen(argv[2]) < 3)
    missingDotSlash = 1;
  else if ((argv[2][0] != '/' && argv[2][0] != '~') &&
           (argv[2][0] != '.' || argv[2][1] != '/'))
    missingDotSlash = 1;

  if (missingDotSlash == 0)
    strncpy((char*)inputFileName,argv[2],1024);
  else
  {
    inputFileName[0] = '.';
    inputFileName[1] = '/';
    strncpy((char*)inputFileName+2,argv[2],1022);    
  }



  const char * outputFileName = argv[3];
  timeoutPeriod = atol(argv[4]);


  missingDotSlash = 0;
  if (strlen(argv[1]) < 3)
    missingDotSlash = 1;
  else if ((argv[1][0] != '/' && argv[1][0] != '~') &&
           (argv[1][0] != '.' || argv[1][1] != '/'))
    missingDotSlash = 1;
   
  if (missingDotSlash == 0)
    strncpy((char*)execname,argv[1],1024);
  else
  {
    execname[0] = '.';
    execname[1] = '/';
    strncpy((char*)execname+2,argv[1],1022);    
  }


  struct stat tmpstat;
  if (stat(execname,&tmpstat) != 0)
  {
    printf("Specified executable file does not exist\n");
    exit(255);
  }
  if (stat((char*)inputFileName,&tmpstat) != 0)
  {
    printf("Input file does not exist\n");
    exit(255);
  }


  stdoutLog = (char*)malloc(2000000);

  int len = strlen(execname);
  isJava = 0;

  if (len >= 7)
    if (strcmp(".class",execname+len-6) == 0)
    {
      isJava = 1;
      int offset = 0;
      if (len > offset+2 && execname[offset] == '.' && execname[offset+1] == '/')
        offset += 2;

      strncpy((char*)classname,execname+offset,2048);
      classname[len-offset-6] = '\0';
    }



  spawnChild(execname,0);
  
  int numfds = 3;

  struct pollfd fds[3];
  int fdtype[3] = {0};
  int fdownerindex[3] = {0};

  fds[0].fd = children[0].fdStdout;
  fds[0].events = events;
  fdtype[0] = 1;
  fdownerindex[0] = 0;

  fds[1].fd = children[0].fdStderr;
  fds[1].events = events;
  fdtype[1] = 2;
  fdownerindex[1] = 0;

  fds[2].fd = selfpipe[0];
  fds[2].events = events;
  fdtype[2] = 0;
  fdownerindex[2] = -1;
  


  int exitQueue[DEAD_QUEUE_SIZE] = {0};
  int exitQueueStartPos = 0;
  int exitQueueEndPos = 0;

  int polltimems = 1000;

  while (childIsAlive != 0)
  {
    //Poll, and if interrupted by
    //a signal, try again	
    if (pollRetryOnSignal(fds,numfds,polltimems) < 0)
    {
      printf("poll() failed\n");
      exit(255);
    }

    //Set poll time to the standard value
    polltimems = 1000;

    for (i = 0; i < numfds; ++i)
    {
      if (fdtype[i] == 0)
      {
        if ((fds[i].revents & errorEvents) != 0)
        {
          //If there is an error on this stream,
          //remove it from the poll list
          fds[i] = fds[numfds-1];
          fdtype[i] = fdtype[numfds-1];
          fdownerindex[i] = fdownerindex[numfds-1];
          --numfds;
          --i;
        }
        else if ((fds[i].revents & events) != 0)
        {
          read(selfpipe[0], buf, 256);
        }
      }
      else if (fdtype[i] == 1)
      {
        if ((fds[i].revents & events) != 0)
        {
          int index = fdownerindex[i];
          reportStdout(index);
        }

        if ((fds[i].revents & errorEvents) != 0)
        {
          //If there is an error on this stream,
          //remove it from the poll list
          fds[i] = fds[numfds-1];
          fdtype[i] = fdtype[numfds-1];
          fdownerindex[i] = fdownerindex[numfds-1];
          --numfds;
          --i;
        }
      }
      else if (fdtype[i] == 2)
      {
        if ((fds[i].revents & events) != 0)
        {
          int index = fdownerindex[i];
          reportStderr(index);
        }

        if ((fds[i].revents & errorEvents) != 0)
        {
          //If there is an error on this stream,
          //remove it from the poll list
          fds[i] = fds[numfds-1];
          fdtype[i] = fdtype[numfds-1];
          fdownerindex[i] = fdownerindex[numfds-1];
          --numfds;
          --i;
        }
      }      
    }




        //Report all processes on the exit queue and free
        //all resources, and restart the processes if necessary

        for (; exitQueueStartPos != exitQueueEndPos; 
             exitQueueStartPos = (exitQueueStartPos+1) % DEAD_QUEUE_SIZE)    
        {
          i = exitQueue[exitQueueStartPos];

          if (children[i].terminatedBySignal == 0)
          {
            if (isJava != 0 && children[i].exitNum == 1)
            {
              printf("Process terminated abnormally on an uncaught exception at %2.2f\n", 
                     getCurrentTime());

              close(children[i].fdStdout);
              close(children[i].fdStderr);

              childIsAlive = 0;

              exit(1);              
            }

            if (isJava == 0)
            {
              int soapboxLogFile = open("soapbox.log", O_RDONLY);

              int isBlank = 1;

              int n = read(soapboxLogFile, buf, 256); 
              for (; n > 0 && isBlank != 0; 
                   n = read(soapboxLogFile, buf, 256))
              {
                int k;
                for (k = 0; k < n && isBlank != 0; ++k)
                  if (buf[k] > ' ')
                    isBlank = 0;
              }

              close(soapboxLogFile);

              if (isBlank == 0)
              {
                printf("Process terminated abnormally due to a security exception at %2.2f\n", 
                       getCurrentTime());

                close(children[i].fdStdout);
                close(children[i].fdStderr);

                childIsAlive = 0;

                exit(3);              
              }
            }



            printf("Process terminated normally with exit code %d at %2.2f\n", 
                    children[i].exitNum, getCurrentTime());

            close(children[i].fdStdout);
            close(children[i].fdStderr);

            childIsAlive = 0;
          }
          else
          {
            printf("Process terminated abnormally on signal %d at %2.2f\n", 
                   children[i].exitNum, getCurrentTime());

            close(children[i].fdStdout);
            close(children[i].fdStderr);

            childIsAlive = 0;

            exit(1);
          }
        }



        //Enqueue any processes in the dead queue into the
        //exit queue, so that they will be processed in the
        //next iteration, and set the poll time to a low
        //value -- this allows us to check the output pipes
        //of processes that died before we clean up

        if (deadQueueStartPos != deadQueueEndPos)
        {
          polltimems = 10;

          exitQueue[exitQueueEndPos] = deadQueue[deadQueueStartPos];
          exitQueueEndPos = (exitQueueEndPos+1) % DEAD_QUEUE_SIZE;
          deadQueueStartPos = (deadQueueStartPos+1) % DEAD_QUEUE_SIZE;

          for (; deadQueueStartPos != deadQueueEndPos; 
               deadQueueStartPos = (deadQueueStartPos+1) % DEAD_QUEUE_SIZE)    
          {
            exitQueue[exitQueueEndPos] = deadQueue[deadQueueStartPos];
            exitQueueEndPos = (exitQueueEndPos+1) % DEAD_QUEUE_SIZE;
          }
        }

  }



  //Set SIGCHLD to the default handler
  signal(SIGCHLD, SIG_DFL);

  //Close both ends of the self pipe
  close(selfpipe[0]);
  close(selfpipe[1]);




  //Check for abnormal termination





  //Print STDOUT output to the specified file 
  FILE * outfile = fopen(outputFileName,"w");  
  fwrite(stdoutLog,1,stdoutLogLen,outfile);  
  fclose(outfile); 

  free(stdoutLog);

  return 0;
}
