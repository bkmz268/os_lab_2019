#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
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

int main(int argc, char **argv) {
  int sockfd, n;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;

  int SERV_PORT = -1;
  int BUFSIZE = -1;
  char IP[16] = {'\0'};

  while (1) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"bufsize", required_argument, 0, 0},
                                      {"ip",required_argument,0,0},
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
                case 2:
                    strcpy(IP, optarg);
                    if (!(strlen(IP)>0))
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
  if (SERV_PORT == -1 || BUFSIZE == -1 || strlen(IP)==0) {
    fprintf(stderr, "Using: %s --port 20001 --bufsize 4 --ip 127.0.0.1\n", argv[0]);
    return 1;
  }

  char sendline[BUFSIZE], recvline[BUFSIZE + 1];

  //Описываем адрес сокета
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, IP, &servaddr.sin_addr) < 0) {
    perror("inet_pton problem (SOCK_DGRAM)");
    exit(1);
  }
  servaddr.sin_port = htons(SERV_PORT);

  //Возвращает файловый дескриптор(>=0), который будет использоваться как ссылка на созданный коммуникационный узел
  //SOCK_DGRAM – для датаграммных
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket problem (SOCK_DGRAM)");
    exit(1);
  }

  write(1, "Enter string: \n", 13);

  while ((n = read(0, sendline, BUFSIZE)) > 0) {
    //отправляет сообщения в сокет  
    //соединение не обязательно
    if (sendto(sockfd, sendline, n, 0, (SADDR *)&servaddr, SLEN) == -1) {
      perror("sendto problem (SOCK_DGRAM)");
      exit(1);
    }
    sleep(1);

    /*виснет, если не получили ответ от сервера
    //могут использоваться для получения данных, независимо от того, 
    //является ли сокет ориентированным на соединения или нет.
    if (recvfrom(sockfd, recvline, BUFSIZE, 0, NULL, NULL) == -1) {
      perror("recvfrom problem (SOCK_DGRAM)");
      exit(1);
    }
    //ОТВЕТ
    printf("REPLY FROM SERVER= %s\n", recvline);
    */
    
  }

  //Закрывает (или прерывает) все существующие соединения сокета
  close(sockfd);
}
