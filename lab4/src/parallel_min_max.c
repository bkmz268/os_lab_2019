#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>
#include <errno.h>

#include "find_min_max.h"
#include "utils.h"

void KillChild(int signum){

    kill(-1, SIGKILL);
    printf ("processes were successfully killed");
}


int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;
  int time_out = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
              if (seed <= 0) {
                printf("seed is a positive number\n");
                 return 1;
                }
            break;
          case 1:
            array_size = atoi(optarg);
             if (array_size <= 0) {
                printf("array_size is a positive number\n");
                 return 1;
                }
            break;
          case 2:
            pnum = atoi(optarg);
             if (pnum <= 0) {
                printf("pnum is a positive number\n");
                 return 1;
                }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            time_out = atoi(optarg);
                if (time_out <= -1) {
                    printf("timeout is a positive number\n");
                    return 1;
                }
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  int pipefd[2];
  pid_t cpid;
   if (pipe(pipefd) == -1) {
               perror("pipe");
               exit(EXIT_FAILURE);
    }

  struct MinMax min_max_pnum;
  int part_pnum = array_size/pnum;
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        min_max_pnum = GetMinMax(array, i*part_pnum, (i == pnum - 1) ? array_size : (i + 1)*part_pnum);

        if (with_files) {
          FILE* outFile = fopen("min_max_out.txt", "a");
          fwrite(&min_max_pnum, sizeof(struct MinMax), 1, outFile);
          fclose(outFile);
        } else {
          write(pipefd[1], &min_max_pnum, sizeof(struct MinMax));
        }
        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  if (time_out > 0){
        
        signal (SIGALRM, KillChild);
        alarm (time_out);
    }


  while (active_child_processes > 0) {

    close(pipefd[1]);

    int wpid = waitpid(-1, NULL, WNOHANG);

        if(wpid == -1)
        {
            if(errno == ECHILD) break;
        }
        else
        {
            active_child_processes -= 1;
        }
        printf("%d\n", active_child_processes);

  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
        FILE* outFile = fopen("min_max_out.txt", "rb");
        fseek(outFile, i*sizeof(struct MinMax), SEEK_SET);
        fread(&min_max_pnum, sizeof(struct MinMax), 1, outFile);
        fclose(outFile);
    } else {
        read(pipefd[0], &min_max_pnum, sizeof(struct MinMax));
    }
    
    
    if (min_max_pnum.min < min_max.min)
        min_max.min = min_max_pnum.min;
    if (min_max_pnum.max > min_max.max) 
        min_max.max = min_max_pnum.max;
  }
  

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
  return 0;
}