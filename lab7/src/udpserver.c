#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>

//#define SERV_PORT 20001
//#define BUFSIZE 1024
#define SADDR struct sockaddr
#define SLEN sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
  int sockfd, n;
  int sockfd2 = 0;
  
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  int SERV_PORT = -1;
  int BUFSIZE = -1;
  while (1) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"bufsize", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1)
        break;
    switch (c) {
        case 0: {
            switch (option_index) {
            case 0:
                SERV_PORT = atoi(optarg);
                if (!(SERV_PORT>0))
                    return 0;
                break;
            case 1:
                BUFSIZE = atoi(optarg);
                if (!(BUFSIZE>0))
                    return 0;
                break;
            default:
                printf("Index %d is out of options\n", option_index);
            }
        } break;
        case '?':
            printf("Unknown argument\n");
            break;
        default:
            fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }
  if (SERV_PORT == -1 || BUFSIZE == -1) {
    fprintf(stderr, "Using: %s --port 20001 --bufsize 4\n", argv[0]);
    return 1;
  }
  char mesg[BUFSIZE], ipadr[16];

  //Возвращает файловый дескриптор(>=0), который будет использоваться как ссылка на созданный коммуникационный узел
  //SOCK_DGRAM – для датаграммных
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem (SOCK_DGRAM)");
    exit(1);
  }

  //параметры для настройки адреса сокета
  memset(&servaddr, 0, SLEN);
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  //настройка адреса сокета
  if (bind(sockfd, (SADDR *)&servaddr, SLEN) < 0) {
    perror("bind problem (SOCK_DGRAM)");
    exit(1);
  }
  printf("SERVER starts...\n");

  //Слушаем в цикле
  int flag = 1;
  while (1) {
    if (sockfd2 != 0) {
        sockfd = sockfd2;
    }
    unsigned int len = SLEN;

    //могут использоваться для получения данных, независимо от того, 
    //является ли сокет ориентированным на соединения или нет.
    if ((n = recvfrom(sockfd, mesg, BUFSIZE, 0, (SADDR *)&cliaddr, &len)) < 0) {
      perror("recvfrom problem (SOCK_DGRAM)");
      exit(1);
    }
    mesg[n] = 0;

    //ЗАПРОС
    printf("REQUEST %s      FROM %s : %d\n", mesg,
           inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, ipadr, 16),
           ntohs(cliaddr.sin_port));

    //отправляет сообщения в сокет  
    //соединение не обязательно
    if (sendto(sockfd, mesg, n, 0, (SADDR *)&cliaddr, len) < 0) {
      perror("sendto problem (SOCK_DGRAM)");
      exit(1);
    }

    if(flag) {
        // падение сервера = сокет не может принимать данные
        shutdown(sockfd, SHUT_RDWR);
        close(sockfd);

        sleep(1);

        if ((sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket problem (SOCK_DGRAM)");
            exit(1);
        }
        if (bind(sockfd2, (SADDR *)&servaddr, SLEN) < 0) {
            perror("bind problem");
            exit(1);
        }
        flag = -1;
    }

  }
}
