#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#define no_argument 0
#define required_argument 1 
#define optional_argument 2

#define RDONLY  'a'
#define WRONLY  'b'
#define COMMAND 'c'
#define VERBOSE 'd'
#define WAIT    'e'

int main(int argc, char * argv[])
{
  const struct option longopts[] =
  {
    {"rdonly",  required_argument, 0, RDONLY },
    {"wronly",  required_argument, 0, WRONLY },
    {"command", required_argument, 0, COMMAND},
    {"verbose", no_argument,       0, VERBOSE},
    {"wait",    no_argument,       0, WAIT},
    {0,           0,               0,  0   }  //denote end
  };
  struct wait_info
  {
    int childPid;
    char* args[20];
  };
  //an array of wait information which is used in wait case to print out exit status and command of each subprocess
  struct wait_info waits[20];
  int numSubPro=0;  //number of subprocess

  int index;
  int iarg=0;
  int fd;
  pid_t pid;
  //turn off getopt error message
  //opterr=0; 
  errno = 0;
  int verbose_shown=0;
  int logic_fd=0;
  //map logic fd to real fd
  int fd_vec[argc];
  while((iarg = getopt_long(argc, argv, "", longopts, &index)) != -1)
  {
    if(verbose_shown==1){
      printf ("--%s ", longopts[index].name);
      //Does this command has argument?
      if (optarg)
      { 
		int vidx;
        for(vidx = optind-1;vidx < argc; ++vidx){
          if(strncmp(argv[vidx], "--", 2) == 0)
              break;  //hit the next option
          printf("%s ", argv[vidx]);
        }
        printf("\n");

      }
    }
    switch (iarg)
    {
      case WAIT:
      {
        //printf("%d\n",numSubPro);
        int i;
        for(i=0;i<numSubPro;i++){
          //call wait
          int status;
          waitpid(waits[i].childPid,&status,0);
          printf("%d", WEXITSTATUS(status));
          int j;
          for(j=0;waits[i].args[j]!=NULL;j++)
            printf(" %s", waits[i].args[j]);
          printf("\n");
        }
      }
      case VERBOSE:
      {
        verbose_shown=1;
        break;
      }
      case COMMAND:
      {
        pid = fork();
        char* a[20];
        // Child process
        if (pid == 0) {
            // Execute command
            if(optarg){
              optind--;
              //printf("first logistic fd: %s, real fd is:%d\n", argv[optind],fd_vec[atoi(argv[optind])]);
              dup2(fd_vec[atoi(argv[optind++])],0);
              //printf("second logistic fd: %s, real fd is:%d\n", argv[optind],fd_vec[atoi(argv[optind])]);
              dup2(fd_vec[atoi(argv[optind++])],1);
              //printf("third logistic fd: %s, real fd is:%d\n", argv[optind],fd_vec[atoi(argv[optind])]);
              dup2(fd_vec[atoi(argv[optind++])],2);
              //optind++;
              //optind++;
              //printf("optind: %d, argc: %d\n", optind,argc);
              int i;
              for (i = 0;optind < argc; ++i,++optind)
              {
                if(strncmp(argv[optind], "--", 2) == 0)
                  break;  //hit the next option
                a[i] = argv[optind];
                //printf("%s\n", a[i]);
              }
              a[i] = NULL;
              execvp(a[0], a);  //if it executed successfully, it would return to parent process
              //if execvp returns, it must have failed
              fprintf(stderr,"Failed with error [%s]\n",strerror(errno));
              exit(0);
              //exit(errno);
              //perror(a[0]);

            }  
        }
        // Parent process
        else {
            // Wait for child process to finish
            int childStatus;
            struct wait_info wait;
            wait.childPid = waitpid(-1, &childStatus, 0);
            //store argument of this command
            int i,arg_idx;
            for (i = 0,arg_idx = optind+2;arg_idx < argc; ++i,++arg_idx)
            {
              if(strncmp(argv[arg_idx], "--", 2) == 0)
                break;  //hit the next option
              wait.args[i] = argv[arg_idx];
              //printf("%s\n", wait.args[i]);
            }
            wait.args[i] = NULL;
            waits[numSubPro++] = wait;
            //return 1;
        } 
        break;
      }
      case RDONLY:
      {
        if((fd = open(optarg, O_RDONLY))!=-1){
          //printf("read only is hit, the logic fd is %d\n",logic_fd);
          fd_vec[logic_fd++] = fd;  //store the true fd and make it can be visited via logic fd
        }
        else{
          //printf("\n open() failed with error [%s]\n",strerror(errno));
          //return 1;
          fprintf(stderr, "\n open() failed with error [%s]\n",strerror(errno)); /* open failed */ 
          exit(1);
        }
        break;
      }
      case WRONLY:
      {
        if((fd = open(optarg, O_WRONLY))!=-1){
          //printf("write only is hit, the logic fd is %d\n",logic_fd);
          fd_vec[logic_fd++] = fd;  //store the true fd and make it can be visited via logic fd
        }
        else{
          fprintf(stderr, "\n open() failed with error [%s]\n",strerror(errno)); /* open failed */ 
          exit(1);
        }
        break;
      }
    }
  }

  return 0; 
}