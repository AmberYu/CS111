#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#define no_argument 0
#define required_argument 1 
#define optional_argument 2

#define RDONLY    'a'
#define WRONLY    'b'
#define COMMAND   'c'
#define VERBOSE   'd'
#define APPEND    'e'
#define CLOEXEC   'f'
#define CREAT     'g'
#define DIRECTORY 'h'
#define DSYNC     'i'
#define EXCL      'j'
#define NOFOLLOW  'k'
#define NONBLOCK  'l'
#define RSYNC     'm'
#define SYNC      'n'
#define TRUNC     'o'
#define RDWR      'p'
#define PIPE      'q'
#define WAIT      'r'
#define CLOSE     's'
#define ABORT     't'
#define CATCH     'u'
#define IGNORE    'v'
#define DEFAULT   'w'
#define PAUSE     'x'

int main(int argc, char * argv[])
{
  const struct option longopts[] =
  {
    {"rdonly",  required_argument, 0, RDONLY },
    {"wronly",  required_argument, 0, WRONLY },
    {"command", required_argument, 0, COMMAND},
    {"verbose", no_argument,       0, VERBOSE},
    {"append",  no_argument, 0, APPEND },
    {"cloexec",  no_argument, 0, CLOEXEC },
    {"creat", no_argument, 0, CREAT},
    {"directory", no_argument,       0, DIRECTORY},
    {"dsync",  no_argument, 0, DSYNC },
    {"excl",  no_argument, 0, EXCL },
    {"nofollow", no_argument, 0, NOFOLLOW},
    {"nonblock", no_argument,       0, NONBLOCK},
    {"rsync",  no_argument, 0, RSYNC },
    {"sync",  no_argument, 0, SYNC },
    {"trunc", no_argument, 0, TRUNC},
    {"rdwr", required_argument, 0, RDWR},
    {"pipe", no_argument, 0, PIPE},
    {"wait", no_argument, 0, WAIT},
    {"close", required_argument, 0, CLOSE},
    {"abort", no_argument, 0, ABORT},
    {"catch", required_argument, 0, CATCH},
    {"ignore", required_argument, 0, IGNORE},
    {"default", required_argument, 0, DEFAULT},
    {"pause", no_argument, 0, PAUSE},
    {0,           0,               0,  0   }  //denote end
  };
  //keep record of wait information in parent process for future use when come accross wait option
  struct wait_info
  {
    int childPid;
    char* args[20];
  };
  //an array of wait information which is used in wait case to print out exit status and command of each subprocess
  struct wait_info waits[20];
  int numSubPro=0;  //number of subprocess

  int oflag=0;   //record the mode of open
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

              int j = 3;
              for(j; j < argc; ++j)
              {
                close(fd_vec[j]);
              }


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

      case PIPE:
      {
          int pipefd[2];
          // printf("1\n");
          pipe(pipefd);
          // printf("1\n");
          fd_vec[logic_fd++] = pipefd[0];  //read end of pipe
          // printf("%d\n", pipefd[0]);
          // printf("%d\n", logic_fd);
          fd_vec[logic_fd++] = pipefd[1];  //write end of pipe
          // printf("%d\n", pipefd[1]);
          // printf("%d\n", logic_fd);
          break;
      }

      case APPEND:
      {
        printf("append called\n");
        oflag|=O_APPEND;
        break;
      }
      case CLOEXEC:
      {
        oflag|=O_CLOEXEC;
        break;
      }
      case CREAT:
      {
        printf("creat called\n");
        oflag|=O_CREAT;
        break;
      }
      case DIRECTORY:
      {
        oflag|=O_DIRECTORY;
        break;
      }
      case DSYNC:
      {
        oflag|=O_DSYNC;
        break;
      }
      case EXCL:
      {
        oflag|=O_EXCL;
        break;
      }
      case NOFOLLOW:
      {
        oflag|=O_NOFOLLOW;
        break;
      }
      case NONBLOCK:
      {
        oflag|=O_NONBLOCK;
        break;
      }
      // case RSYNC:
      // {
      //   oflag|=O_RSYNC;
      //   break;
      // }
      
      case SYNC:
      {
        oflag|=O_SYNC;
        break;
      }
      case TRUNC:
      {
        oflag|=O_TRUNC;
        break;
      }
      case RDONLY:
      {
        oflag|=O_RDONLY;
        if((fd = open(optarg, oflag))!=-1){
          //printf("read only is hit, the logic fd is %d\n",logic_fd);
          fd_vec[logic_fd++] = fd;  //store the true fd and make it can be visited via logic fd
        }
        else{
          //printf("\n open() failed with error [%s]\n",strerror(errno));
          //return 1;
          fprintf(stderr, "\n open() failed with error [%s]\n",strerror(errno)); /* open failed */ 
          exit(1);
        }
        oflag=0;
        break;
      }
      case WRONLY:
      {
        oflag|=O_WRONLY;
        if((fd = open(optarg, oflag,0644))!=-1){
          //printf("write only is hit, the logic fd is %d\n",logic_fd);
          fd_vec[logic_fd++] = fd;  //store the true fd and make it can be visited via logic fd
        }
        else{
          fprintf(stderr, "\n open() failed with error [%s]\n",strerror(errno)); /* open failed */ 
          exit(1);
        }
        oflag=0;
        break;
      }
    }
  }

  return 0; 
}