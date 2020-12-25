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

int main(int argc, char *argv[]) {
  const size_t kSize = sizeof(struct sockaddr_in);
  int fd_tcp, fd_udp;
  int nread;
  struct sockaddr_in servaddr_tcp, servaddr_udp;
  struct sockaddr_in cliaddr;
  int buf_size = -1;
  int port_tcp = -1;
  int port_udp = -1;

  while (1) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"buf_size", required_argument, 0, 0},
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
        if ((port_tcp = atoi(optarg)) == 0) {
          printf("Error: bad port_tcp value\n");
          return -1;
        }
        break;
    case 2:
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

  if (buf_size == -1 || port_tcp == -1) {
    fprintf(stderr, "Using: %s --buf_size [NUMBER] --port_tcp [NUMBER] --port_udp [NUMBER]\n",
            argv[0]);
    return -1;
  }

  char buf[buf_size];

  if ((fd_tcp = socket(AF_INET, SOCK_STREAM, 0)) < 0 || 
      (fd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }
#ifdef VERBOSE
  printf("Socket %d and %d created\n", fd_tcp, fd_udp);
#endif

  memset(&servaddr_tcp, 0, kSize);
  memset(&servaddr_udp, 0, kSize);
  servaddr_tcp.sin_family = AF_INET;
  servaddr_tcp.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr_tcp.sin_port = htons(port_tcp);
  servaddr_udp.sin_family = AF_INET;
  servaddr_udp.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr_udp.sin_port = htons(port_udp);

  int opt_val = 1;
  setsockopt(fd_tcp, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
  setsockopt(fd_udp, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  if (bind(fd_tcp, (SADDR *)&servaddr_tcp, kSize) < 0 ||
      bind(fd_udp, (SADDR *)&servaddr_udp, kSize) < 0) {
    perror("bind");
    exit(1);
  }

#ifdef VERBOSE
  printf("Socket %d bound to %d:%d\n", fd_tcp, servaddr_tcp.sin_addr.s_addr, ntohs(servaddr_tcp.sin_port));
#endif

  if (listen(fd_tcp, 5) < 0) {
    perror("listen");
    exit(1);
  }

#ifdef VERBOSE
  printf("Socket %d is listening\n", fd_tcp);
#endif

  while (1) {
    unsigned int clilen = kSize;

    if (accept(fd_tcp, (SADDR *)&cliaddr, &clilen) < 0) {
      perror("accept");
      printf("Accept error: %d\n", errno);
      exit(1);
    }


#ifdef VERBOSE
    printf("Connection established, client: %d [%d:%d]\n", fd_udp, ntohl(cliaddr.sin_addr.s_addr), ntohs(cliaddr.sin_port));
#endif
    while ((nread = recvfrom(fd_udp, buf, buf_size, 0, (SADDR *)&cliaddr, &clilen)) > 0) {
      write(1, &buf, nread);
    }

    if (nread == -1) {
      perror("read");
      exit(1);
    }
  }
  close(fd_udp);
  close(fd_tcp);
}
