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


void sig_handler(sigNum)
{

}


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

  int index;
  int iarg=0;
  int fd;
  pid_t pid;
  //turn off getopt error message
  //opterr=0; 
  errno = 0;
  int verbose_shown=0;
  char* s[20];   //s[0] is a string
  int logic_fd=0;
  int oflag=0;   //record the mode of open
  //map logic fd to real fd
  int fd_vec[argc];
  int wait_index = 0;
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
      case VERBOSE:
      {
        verbose_shown=1;
        break;
      }

      case WAIT:
      {

        //--wait, as each subprocess finishes, it prints its exit status, followed by the command and arguments
        //print exit status
          
        // count = num_of_subprocess
        int i = 0;
        for(i ; i < count; ++i)
        {
          //wait for the subprocess to return 
          wait();

        }    
        // if(!s.empty())
        // {

          for(int i = 0 ; s[i] != NULL; ++i)
          {
            printf("%s ", s[i]);
          }
          
        // }
                
                
                 
          
        break;
      }

      case APPEND:
      {
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
        if((fd = open(optarg, oflag))!=-1){
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

      case RDWR:
      {
        oflag|=O_RDWR;
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


      case CLOSE:
      {
        close(fd_vec(optarg));
        break;
      }

      case ABORT:
      {
        raise(SIGSEGV);
        break;
      }

      case CATCH:
      {
        signal(optarg, sig_handler(optarg));
        break;
      }

      case IGNORE:
      {
        signal(optarg, SIG_IGN);
        break;
      }

      case DEFAULT:
      {
        signal(optarg, SIG_DEL);
        break;
      }

      case PAUSE:
      {
        signal(optarg, SIG_DEL);
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
         

              int i = 0;
              for(i; i < argc; ++i)
              {
                close(fd_vec[i]);
              }

    
              int i = 0;
              for (i;optind < argc; ++i,++optind)
              {
                if(strncmp(argv[optind], "--", 2) == 0)
                  break;  //hit the next option
                a[i] = argv[optind];
                //printf("%s\n", a[i]);

              }
              a[i] = NULL;


              execvp(a[0], a);  //if it executed successfully, it would return to parent process
              

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
            waitpid(pid, &childStatus, 0);

            int exit_status = WEXITSTATUS(childStatus);

            s[wait_index++] = exit_status;

            int i = 0;
            for(i,wait_index; a[i]!= NULL; ++i, ++wait_index)
            {
              s[wait_index] = a[i];
            }
            s[wait_index++] = "/n";
        } 
        break;
      }
    }
  }

  return 0; 
}