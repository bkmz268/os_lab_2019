#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "factorial.h"

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }
  if (errno != 0)
    return false;
  *val = i;
  return true;
}

struct Server {
  char ip[255];
  int port;
};

struct ArgsForParallelServer{
    int begin;
    int end;
    int mod;
    struct Server to_server;
};

int result = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
void ParallelServer(void *args) {
    struct ArgsForParallelServer *thread_args = (struct ArgsForParallelServer *)args;

    struct hostent *hostname = gethostbyname((*thread_args).to_server.ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", (*thread_args).to_server.ip);
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((*thread_args).to_server.port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(1);
    }

   
    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        //fprintf(stderr, "error creating connection: %s ", strerror(errno));
        exit(1);
    }
    printf("%s:%d connect\n", (*thread_args).to_server.ip, (*thread_args).to_server.port);
    
   
    uint64_t begin = (*thread_args).begin;
    uint64_t end = (*thread_args).end;
    uint64_t mod = (*thread_args).mod;

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
      fprintf(stderr, "Send failed\n");
      exit(1);
    }
    printf("%s:%d send: ( %lu, %lu ) mod = %lu\n", (*thread_args).to_server.ip, (*thread_args).to_server.port, begin, end, mod);

  
    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }

    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    printf("%s:%d answer: %lu\n", (*thread_args).to_server.ip, (*thread_args).to_server.port, answer);
    
   
    pthread_mutex_lock(&mut);
    int temp = result;
    result = (temp * answer) % mod;
    pthread_mutex_unlock(&mut);
   
    
    close(sck);
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; 
  while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    bool f;
    if (c == -1)
      break;
    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        f = ConvertStringToUI64(optarg, &k);
        if (!f) {
            printf("incorrect value k\n");
            return -1;
        }
        break;
      case 1:
        f = ConvertStringToUI64(optarg, &mod);
        if (!f) {
            printf("incorrect value mod\n");
            return -1;
        }
        break;
      case 2:
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;
    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
    return 1;
  }

  FILE* addresses = fopen(servers, "r");
  if (addresses == NULL) {
    printf("cannot read addresses");
    return -1;
  }

  char buf[256];
  unsigned int servers_num = 0;
  while(fgets(buf,256,addresses) != NULL)
    servers_num++;
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  fclose(addresses);

  FILE* addresses2 = fopen(servers, "r");
  int i = 0;
  while (!feof(addresses2)) {
    fgets(buf, sizeof(buf), addresses2);
    strcpy((*(to+i)).ip, strtok(buf,":"));
    (*(to+i)).port = atoi(strtok(NULL, ":"));
    i++;
  }
  fclose(addresses2);

  for (i = 0; i < servers_num; i++) {
    printf("Client find %s:%d\n", (*(to+i)).ip, (*(to+i)).port);
  }

  sleep(1);

  pthread_t threads[servers_num];
  struct ArgsForParallelServer args[servers_num];
  for (i = 0; i < servers_num; i++) {
    args[i].to_server = *(to+i);
    args[i].begin = (k / servers_num) * i + 1;
    args[i].end = (k / servers_num) * (i + 1);   
    args[i].mod = mod;
    if (pthread_create(&threads[i], NULL, (void *)ParallelServer, (void *)(args+i))) {
        printf("Error: pthread_create failed!\n");
        return 1;
    }
  }

  for (i = 0; i < servers_num; i++) 
    pthread_join(threads[i], NULL);

  printf("---------------------------\nResult: %d\n", result);
  free(to);
  return 0;
}
