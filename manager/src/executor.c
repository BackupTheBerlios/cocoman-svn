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



int exitQueueSize = 0;
int deadQueueSize = 0;
volatile char inputFileName[1025] = {0};
char * stdoutLog;
int stdoutLogLen = 0;
char * stderrLog;
int stderrLogLen = 0;
volatile int isJava = 0;
volatile char classname[2049] = {0};
extern int errno;          /* for reporting additional error information */
struct timeval startTime;
double timeoutPeriod = 30.0;
int javaException = 0;
int javaSecurityException = 0;


struct childInfo
{
  char * processName;

  int fdStdout;
  int fdStderr;
  int pid;
  int running;

  int terminatedBySignal; 

  //0 - normal termination
  //1 - abnormally terminated due to signal or uncaught exception
  //2 - timed out
  //3 - abnormally terminated due to security violation
  int status;

  //if terminatedBySignal is 0, exitNum is the return code
  //if terminatedBySignal is 1, exitNum is the signal number
  int exitNum;

  double exitTime;
};

volatile struct childInfo child;
volatile int childIsAlive = 0;
int selfpipe[2];






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





void spawnChild(const char * filename)
{
      if (child.running != 0)
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

        child.processName = (char*)filename;
        child.fdStdout = fd1[0];
        child.fdStderr = fd2[0];
        child.pid = pid;
        child.terminatedBySignal = 0;
        child.exitNum = 0;
        child.running = 1;
        child.status = 0;

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
        dup2(fdStdin,0);
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

    
    if (child.running == 1 && pid == child.pid)
    {
      at_least_one = 1;
      child.running = 0;
      deadQueueSize = 1;

      if (WIFSIGNALED(rv))
      {
        child.terminatedBySignal = 1;
        child.exitNum = WTERMSIG(rv);
      }
      else
      {
        child.terminatedBySignal = 0;
        child.exitNum = WEXITSTATUS(rv);
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
              kill(child.pid,SIGINT);
              child.status = 2;
              timeout = 0;
            }

            while ((rc = poll(fds,numfds,timeout)) == -1)
            {
              if (getCurrentTime() > timeoutPeriod)
              {
                kill(child.pid,SIGINT);
                child.status = 2;
                timeout = 0;
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






void reportStdout()
{
          int n = read(child.fdStdout, buf, 256);
          if (n > 0)
          {
            if (stdoutLogLen < 1999000)
            {
              memcpy(stdoutLog+stdoutLogLen,buf,n);
              stdoutLogLen += n;
              stdoutLog[stdoutLogLen] = '\0';
            }

            struct pollfd fds[1];
            fds[0].fd = child.fdStdout;
            fds[0].events = events;
            while (pollRetryOnSignal(fds,1,0) >= 0)
            {
              if ((fds[0].revents & events) != 0)
              {
                int n = read(child.fdStdout, buf, 256); 
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









void reportStderr()
{
          int n = read(child.fdStderr, buf, 256);
          if (n > 0)
          {
            if (stderrLogLen < 1999000)
            {
              memcpy(stderrLog+stderrLogLen,buf,n);
              stderrLogLen += n;
              stderrLog[stderrLogLen] = '\0';
            }

            struct pollfd fds[1];
            fds[0].fd = child.fdStderr;
            fds[0].events = events;
            while (pollRetryOnSignal(fds,1,0) >= 0)
            {
              if ((fds[0].revents & events) != 0)
              {
                int n = read(child.fdStderr, buf, 256); 
                if (n <= 0) break;

                if (stderrLogLen < 1999000)
                {
                  memcpy(stderrLog+stderrLogLen,buf,n);
                  stderrLogLen += n;
                  stderrLog[stderrLogLen] = '\0';
                }
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
  stderrLog = (char*)malloc(2000000);


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



  spawnChild(execname);
  
  int numfds = 3;

  struct pollfd fds[3];
  int fdtype[3] = {0};
  int fdownerindex[3] = {0};

  fds[0].fd = child.fdStdout;
  fds[0].events = events;
  fdtype[0] = 1;
  fdownerindex[0] = 0;

  fds[1].fd = child.fdStderr;
  fds[1].events = events;
  fdtype[1] = 2;
  fdownerindex[1] = 0;

  fds[2].fd = selfpipe[0];
  fds[2].events = events;
  fdtype[2] = 0;
  fdownerindex[2] = -1;
  

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
    if (getCurrentTime() < timeoutPeriod)
      polltimems = 1000;
    else
      polltimems = 0;

    int i;
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

      if (exitQueueSize != 0)
      {
        //Mark the the child is completely dead and that all its
        //output has been logged
        exitQueueSize = 0;
        childIsAlive = 0;

        //Close child's STDOUT and STDERR
        close(child.fdStdout);
        close(child.fdStderr);

        //Record the time at which the child exitted
        child.exitTime = getCurrentTime();

        //If the program did not time out, we need 
        //to check whether it crashed, and we need to
        //check if it was terminated for a security
        //violation
        if (child.status != 2) 
        {
          //Handle programs that crashed
          if (child.terminatedBySignal != 0)
            child.status = 1;

          //Handle Java programs
          else if (isJava != 0)
          {
            //If JVM returned 1
            if (child.exitNum == 1)
            {
              //Check STDERR to see if the program exited
              //due to an uncaught exception

              int n = strstr(stderrLog, "java.lang.SecurityException") - stderrLog;
              if (n >= 0 && n < stderrLogLen)
                child.status = 3;
              else 
              {
                n = strstr(stderrLog, "java.security.AccessControlException") - stderrLog;
                if (n >= 0 && n < stderrLogLen)
                  child.status = 3;
                else
                {
                  n = strstr(stderrLog, "Exception in thread ") - stderrLog;
                  if (n >= 0 && n < stderrLogLen)
                    child.status = 1;
                }
              }
            }
          }

          //Handle C/C++ programs
          else
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
              child.status = 3;
          }
        }
      }



      //Enqueue any processes in the dead queue into the
      //exit queue, so that they will be processed in the
      //next iteration, and set the poll time to a low
      //value -- this allows us to check the output pipes
      //of processes that died before we clean up

      if (deadQueueSize != 0)
      {
        if (polltimems != 0)
          polltimems = 10;
        exitQueueSize = 1;
        deadQueueSize = 0;
      }
  }



  //Set SIGCHLD to the default handler
  signal(SIGCHLD, SIG_DFL);

  //Close both ends of the self pipe
  close(selfpipe[0]);
  close(selfpipe[1]);




  //Check for abnormal termination
  if (child.status != 0)
  {
    if (child.status == 1)
    {
      if (child.terminatedBySignal != 0)
        printf("Program terminated abnormally due to signal %d at time %3.2f\n",
               child.exitNum, child.exitTime);
      else
        printf("Program terminated abnormally due to uncaught exception at time %3.2f\n",
               child.exitTime);
    }
    else if (child.status == 2)
      printf("Program exceeded %3.2f second time limit\n", timeoutPeriod);
    else if (child.status == 3)
      printf("Program was terminated due to a security violation at time %3.2f\n",
             child.exitTime);
    else
      printf("Program terminated for unknown reasons\n");
  }
  else
  {
    printf("Program terminated normally with return code %d at time %3.2f\n",
           child.exitNum, child.exitTime);

    //Print STDOUT output to the specified file 
    FILE * outfile = fopen(outputFileName,"w");  
    fwrite(stdoutLog,1,stdoutLogLen,outfile);  
    fclose(outfile); 
  }

  free(stdoutLog);
  free(stderrLog);

  return child.status;
}
