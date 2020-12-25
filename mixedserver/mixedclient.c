#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>


#define VERBOSE
#define SADDR struct sockaddr
#define SOCKADDR_SIZE sizeof(struct sockaddr_in)

int main(int argc, char *argv[]) {
  int fd_tcp, fd_udp;
  int nread;
  int buf_size = -1;
  int port_tcp = -1;
  int port_udp = -1;
  char ip[16] = {'\0'};
  struct sockaddr_in servaddr_tcp, servaddr_udp;

  while (1) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"buf_size", required_argument, 0, 0},
                                      {"ip", required_argument, 0, 0},
                                      {"port_tcp", required_argument, 0, 0},
                                      {"port_udp", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        if ((buf_size = atoi(optarg)) == 0) {
          printf("Error: bad buf_size value\n");
          return -1;
        }
        break;
      case 1:
        strcpy(ip, optarg);
        break;
      case 2:
        if ((port_tcp = atoi(optarg)) == 0) {
          printf("Error: bad port_tcp value\n");
          return -1;
        }
        break;
    case 3:
        if ((port_udp = atoi(optarg)) == 0) {
          printf("Error: bad port_udp value\n");
          return -1;
        }
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

  if (buf_size == -1 || port_tcp == -1 || strlen(ip) == 0 || port_udp == -1) {
    fprintf(stderr, "Using: %s --buf_size [NUMBER] --port_tcp [NUMBER] --port_udp [NUMBER] --ip [IPADDR]\n",
            argv[0]);
    return -1;
  }

#ifdef VERBOSE
  printf("buf=%d port_tcp=%d port_udp=%d ip=%s\n", buf_size, port_tcp, port_udp, ip);
#endif
  char buf[buf_size];

  if ((fd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0 ||
      (fd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creating");
    exit(1);
  }

#ifdef VERBOSE
  printf("Socket %d and %d created\n", fd_tcp, fd_udp);
#endif

  memset(&servaddr_tcp, 0, SOCKADDR_SIZE);
  memset(&servaddr_udp, 0, SOCKADDR_SIZE);
  servaddr_tcp.sin_family = AF_INET;
  servaddr_udp.sin_family = AF_INET;
  
  if (inet_pton(AF_INET, ip, &servaddr_tcp.sin_addr) <= 0 ||
      inet_pton(AF_INET, ip, &servaddr_udp.sin_addr) <= 0) {
    perror("bad address");
    exit(1);
  }

  servaddr_tcp.sin_port = htons(port_tcp);
  servaddr_udp.sin_port = htons(port_udp);

  if (connect(fd_tcp, (SADDR *)&servaddr_tcp, SOCKADDR_SIZE) < 0) {
    perror("connect");
    printf("Connect error: %d\n", errno);
    exit(1);
  }

  write(1, "Input message to send\n", 22);
  while ((nread = read(0, buf, buf_size)) > 0) {
    if (sendto(fd_udp, buf, nread, 0, (SADDR *)&servaddr_udp, SOCKADDR_SIZE) < 0) {
      perror("send");
      exit(1);
    }
  }

  close(fd_udp);
  close(fd_tcp);
  exit(0);
}
