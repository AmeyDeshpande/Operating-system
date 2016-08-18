/*Nama - Amey Deshpande*/


using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string>
#include <limits.h>
#include <sys/time.h>

#include "helper-routines.h"

pid_t childpid, parentpid;
pid_t grpidc, grpidp;
timeval t1, t2, start, end, rt1, rt2;
int numtests;
double elapsedTime = 0,totalTripTime = 0,round_trip_time = 0,min_time = 0,max_time = 0;
bool flag = true;
sigset_t sigmask;

void sigusr1_handler(int sig)
{	
  gettimeofday(&rt2, NULL);
  round_trip_time = (rt2.tv_sec - rt1.tv_sec)*1000.0;
  round_trip_time += (rt2.tv_usec - rt1.tv_usec)/1000.0;
  totalTripTime += round_trip_time;
  if(round_trip_time > max_time || max_time==0)
    max_time = round_trip_time;
  if(round_trip_time < min_time || min_time == 0)
    min_time = round_trip_time;
  gettimeofday(&rt1, NULL);
  kill(getppid(), SIGUSR2);
}
void sigusr2_handler(int sig)
{
  gettimeofday(&rt2, NULL);
  round_trip_time = (rt2.tv_sec - rt1.tv_sec)*1000.0;
  round_trip_time += (rt2.tv_usec - rt1.tv_usec)/1000.0;
  totalTripTime += round_trip_time;
  if(round_trip_time > max_time || max_time==0)
    max_time = round_trip_time;
  if(round_trip_time < min_time || min_time==0)
    min_time = round_trip_time;
  gettimeofday(&rt1, NULL);
  kill(childpid, SIGUSR1); 
}
void sigstp_handler(int sig)
{
  flag = false;
}
int main(int argc, char **argv)
{
  int fd1[2],fd2[2];
  char childmsg[] = "1";
  char parentmsg[] = "2";
  char quitmsg[] = "q";
  char readbuffer [2]; 
  char buffer2 [1025]; 
  int nbytes = 0;
  double min = LONG_MAX, max = LONG_MIN, average = 0;	

  Signal(SIGUSR1,  sigusr1_handler);
  Signal(SIGUSR2,  sigusr2_handler);
  Signal(SIGTSTP, sigstp_handler);

  int numtests;
  if (argv[2])
    numtests= atoi(argv[2]);
  else
    numtests = 10000;
  const int MAX_TESTS = numtests;

  if(argc<2){
    printf("Not enough arguments\n");
    exit(0);
 	}
 	if(pipe(fd1) < 0 || pipe(fd2) < 0)
  {
    printf("%s\n", strerror(errno));
    exit(0);
  }
 	printf("Number of Tests %d\n", numtests);
 	gettimeofday(&t1, NULL);
  if((childpid = fork()) == -1)
  {
    perror("Fork Error");
		exit(1);
  }
 	if(strcmp(argv[1],"-p")==0)
  {
    if (childpid == 0)
		{
      int i;
      for(i=0; i<numtests; i++)
      {
        gettimeofday(&start, NULL);
        if ((nbytes=read(fd2[0], readbuffer, sizeof(readbuffer))))
        {
          close(fd1[0]);
          close(fd2[1]); 
        }
        if(write(fd1[1],childmsg, (strlen(childmsg)+1))) 
        {
          gettimeofday(&end, NULL);
          double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0;
          elapsed_time += (end.tv_usec - start.tv_usec) / 1000.0;
          average += elapsed_time; 
          if(min > elapsed_time)
            min = elapsed_time;
          if(max < elapsed_time)
            max = elapsed_time;
        }
      }
      if ((nbytes=read(fd2[0], readbuffer, sizeof(readbuffer)))) 
      {
        if (readbuffer[0] == 'q') 
        {   					
          gettimeofday(&t2, NULL);
          double elapsed_time = (t2.tv_sec - t1.tv_sec) * 1000.0;
          elapsed_time += (t2.tv_usec - t1.tv_usec) / 1000.0;  
            
          printf("Child's Results for Pipe IPC mechanisms\n%s", buffer2);
            
          printf("Process ID is %d, Group ID is %d\n",(int)getpid(),(int)getgid());
          printf("Round trip times\n");
          printf("Average %.6f\n",average/MAX_TESTS);
          printf("Maximum %.6f\n", max);
          printf("Minimum %.6f\n", min);
          printf("Elapsed Time %f\n", elapsed_time);
          close(fd1[0]); 
          close(fd2[1]); 
          write(fd1[1],buffer2, (strlen(buffer2)+1));
        }
      }
      exit(0);
    }
    else 
    {
      int i;
      for(i=0; i<numtests; i++)
      {
        gettimeofday(&start, NULL);
        write(fd2[1], parentmsg,(strlen(parentmsg)+1));
        if((nbytes = read(fd1[0], readbuffer, sizeof(readbuffer))))
        {
          close(fd1[1]); 
          close(fd2[0]); 
        }
        gettimeofday(&end, NULL);
        double elapsed_time = (end.tv_sec - start.tv_sec) * 1000.0;
        elapsed_time += (end.tv_usec - start.tv_usec) / 1000.0; 
        average += elapsed_time; 
        if(min > elapsed_time)
          min = elapsed_time;
        if(max < elapsed_time)
          max = elapsed_time;  		
      }
      write(fd2[1], quitmsg,(strlen(quitmsg)+1)); 
      if((nbytes = read(fd1[0], buffer2, sizeof(buffer2)))) 
      {   					
        close(fd1[1]); 
        close(fd2[0]); 
      }
      wait(0); 
    }
    gettimeofday(&t2, NULL); 
    printf("Parent's Results for Pipe IPC mechanisms\n");	
    printf("Process ID is %d, Group ID is %d\n",(int)getpid(),(int)getgid());
    printf("Round trip times\n");
    
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;  
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;

		printf("Average %.6f\n",average/MAX_TESTS); 
		printf("Maximum %.6f\n", max);
		printf("Minimum %.6f\n", min);
		printf("Elapsed Time %f\n", elapsedTime);
	}
  else if(strcmp(argv[1],"-s")==0)
  {
    int currentTest = 0;
    if (childpid == 0)
    {
      sigaddset(&sigmask, SIGUSR2);
    }
    else
		{
      sigaddset(&sigmask, SIGUSR1);
      sigaddset(&sigmask, SIGTSTP);
    }
    if(childpid == 0)
    {
      max_time = 0;
      gettimeofday(&rt1, NULL);
      while(flag)
      {
        sigsuspend(&sigmask);
      }
    }
    else
    {
      max_time = 0;
      gettimeofday(&rt1, NULL);
      kill(childpid, SIGUSR1);
      while(currentTest < numtests)
      {
        sigsuspend(&sigmask);
        currentTest++;
      }
      kill(childpid, SIGTSTP);
      wait(0);
    }
		gettimeofday(&t2, NULL);
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;
		elapsedTime += (t2.tv_usec - t1.tv_usec)/1000.0;

    if(childpid == 0)
      printf("Child");
    else
      printf("Parent");
    printf("'s Results for Signal IPC mechanisms\n");
    printf("Process ID is %d, Group ID is %d\n", getpid(),getgid());
    printf("Round trip times\n");
    printf("Average %f\n", elapsedTime/numtests);
    printf("Maximum %f\nMinimum %f\n", max_time, min_time);
    printf("Elapsed Time %f\n", elapsedTime);
    exit(0); 
  }
}