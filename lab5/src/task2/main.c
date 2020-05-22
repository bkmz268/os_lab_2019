#include <errno.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <stdint.h>

#include <getopt.h>

uint32_t result = 1; /* A shared variable for threads */
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
int mod;
struct Args {
  int begin;
  int end;
};

void Factorial(void *args) {
  struct Args *thread_args = (struct Args *)args;
  uint32_t thread_result = 1;
  int i = (*thread_args).begin;
  for (; i <= (*thread_args).end; i++){
      thread_result *= i;
  }

  pthread_mutex_lock(&mut);
  uint32_t temp = result;
  printf("intermediate result %d : thread begins from %d  ends to %d -> result of thread %d\n", temp, (*thread_args).begin, (*thread_args).end, (temp * thread_result) % mod);
  result = (temp * thread_result) % mod;
  pthread_mutex_unlock(&mut);
}

int main(int argc, char **argv) {
  int k = -1;
  int pnum = -1;
  mod = -1;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            k = atoi(optarg);
            if (k <= 0) {
            printf("k is a positive number\n");
            return 1;
            }
            break;
          case 1:
            pnum = atoi(optarg);
            if (pnum < 1) {
            printf("pnum is a positive number\n");
            return 1;
            }
            break;
          case 2:
            mod = atoi(optarg);
            if (mod < 1) {
            printf("mod is a positive number\n");
            return 1;
            }
            break;
          defalut:
            printf("Index %d is out of options\n", option_index);
        }
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

  if (k == -1 || pnum == -1 || mod == -1) {
    printf("Usage: %s --k \"num\" --pnum \"num\" --mod \"num\" \n",
           argv[0]);
    return 1;
  }

  pthread_t threads[pnum];
  struct Args args[pnum];

  int i;
  for (i = 0; i < pnum; i++) {
      args[i].begin = (k / pnum) * i + 1;
      args[i].end = (k / pnum) * (i + 1);
      if (pthread_create(&threads[i], NULL, (void *)Factorial, (void *)(args+i))) {
      printf("Error: pthread_create failed!\n");
      return 1;
      }
  }

  for (i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("\nResult: %d\n", result);

  return 0;
}
