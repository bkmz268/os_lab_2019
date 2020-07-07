#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "pthread.h"
#include "factor.h"

int port = -1;

struct FactorialArgs {
  uint64_t begin;
  uint64_t end;
  uint64_t mod;
};

uint64_t Factorial(const struct FactorialArgs *args) {
  uint64_t ans = 1;
  uint64_t i;
  for (i = (*args).begin; i < (*args).end; i++) 
    ans *= i;
  ans %= (*args).mod;
 // printf("%d (%lu - %lu) result: %lu\n", port, (*args).begin, (*args).end-1, ans);
  return ans;
}

void *ThreadFactorial(void *args) {
  struct FactorialArgs *fargs = (struct FactorialArgs *)args;
  return (void *)(uint64_t *)Factorial(fargs);
}


int main(int argc, char **argv) {
  int tnum = -1;
  while (true) {
    int current_optind = optind ? optind : 1;
    static struct option options[] = {{"port", required_argument, 0, 0},
                                      {"tnum", required_argument, 0, 0},
                                      {0, 0, 0, 0}};
    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);
    if (c == -1)
      break;
    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        port = atoi(optarg);
        // TODO: your code here
        if (port < 0) {
            printf("port is a positive number\n");
            return -1;
        }
        break;
      case 1:
        tnum = atoi(optarg);
        // TODO: your code here
        if (tnum <= 0) {
            printf("tnum is a positive number\n");
            return -1;
        }
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
  if (port == -1 || tnum == -1) {
    fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
    return 1;
  }

  //Socket
  //Параметр domain задает домен соединения: выбирает набор протоколов, которые будут использоваться для создания соединения.(AF_INET	IPv4 протоколы Интернет)
  //Сокет имеет тип type, задающий семантику коммуникации.SOCK_STREAM Обеспечивает создание двусторонних надежных и последовательных потоков байтов , поддерживающих соединения. Может также поддерживаться механизм внепоточных данных.
  //Параметр protocol задает конкретный протокол, который работает с сокетом. Обычно существует только один протокол, задающий конкретный тип сокета в определенном семействе протоколов, в этом случае protocol может быть определено, как 0
  //Возвращает файловый дескриптор(>=0), который будет использоваться как ссылка на созданный коммуникационный узел
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    fprintf(stderr, "Can not create server socket!");
    return 1;
  }

  //Структура sockaddr_in описывает сокет для работы с протоколами IP
  struct sockaddr_in server;
  server.sin_family = AF_INET;
  //Порт (htons,htonl: данные из узлового порядка расположения байтов в сетевой)
  server.sin_port = htons((uint16_t)port);
  //IP-адрес. INADDR_ANY связывает сокет со всеми доступными интерфейсами. 
  server.sin_addr.s_addr = htonl(INADDR_ANY);

  //Установливаем флаги на сокете  
  int opt_val = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

  //Cвязывает локальный сетевой адрес транпортного уровня с сокетом
  int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
  if (err < 0) {
    fprintf(stderr, "Can not bind to socket!");
    return 1;
  }
  
  //Cообщает уровню протокола, что сокет готов к принятию новых входящих соединений
  //Перевод сокета в пассивное (слушающее) состояние и создание очередей сокетов
  err = listen(server_fd, 128); //128 - макс размер очереди
  if (err < 0) {
    fprintf(stderr, "Could not listen on socket\n");
    return 1;
  }

  printf("Server listening at %d\n", port);

  //Слушаем в цикле
  while (true) {
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    //Является блокирующим – он ожидает поступления запроса на соединение
    int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

    if (client_fd < 0) {
      fprintf(stderr, "Could not establish new connection\n");
      continue;
    }

    while (true) {
      unsigned int buffer_size = sizeof(uint64_t) * 3;
      char from_client[buffer_size];
      //Получаем сообщение из сокета клиента в from_client
      int read = recv(client_fd, from_client, buffer_size, 0);

      if (!read)
        break;
      if (read < 0) {
        fprintf(stderr, "Client read failed\n");
        break;
      }
      if (read < buffer_size) {
        fprintf(stderr, "Client send wrong data format\n");
        break;
      }

      pthread_t threads[tnum];
      
      //Разбиваем информацию от клиента
      uint64_t begin = 0;
      uint64_t end = 0;
      uint64_t mod = 0;
      memcpy(&begin, from_client, sizeof(uint64_t));
      memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
      memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

      fprintf(stdout, "%d receive: %lu %lu %lu\n", port, begin, end, mod);

      struct FactorialArgs args[tnum];
      uint32_t i;
      for ( i = 0; i < tnum; i++) {
        // TODO: parallel somehow
        args[i].mod = mod;
        args[i].begin = begin + (end-begin+1)/tnum*i;
        if (tnum%2==1 && i == (tnum-1))
            args[i].end = begin + (end-begin+1)/tnum*(i+1) +1;
        else
            args[i].end = begin + (end-begin+1)/tnum*(i+1);
        //fprintf(stdout, "%d %d - %lu %lu\n", port, i, args[i].begin, args[i].end);
        //Создаём потоки с функцией подсчёта факториала
        if (pthread_create(&threads[i], NULL, ThreadFactorial,
                           (void *)&args[i])) {
          printf("Error: pthread_create failed!\n");
          return 1;
        }
      }
      //Дожидаемся завершения потоков и сводим результат
      uint64_t total = 1;
      for (i = 0; i < tnum; i++) {
        uint64_t result = 0;
        pthread_join(threads[i], (void **)&result);
        total = MultModulo(total, result, mod);
      }

      printf("%d total: %lu\n", port, total);

      //Отправляет сообщения в сокет клиента
      char buffer[sizeof(total)];
      memcpy(buffer, &total, sizeof(total));
      err = send(client_fd, buffer, sizeof(total), 0);
      if (err < 0) {
        fprintf(stderr, "Can't send data to client\n");
        break;
      }
    }

    //Немедленное закрытие всех или части связей на сокет
    shutdown(client_fd, SHUT_RDWR);
    //Закрывает (или прерывает) все существующие соединения сокета
    close(client_fd);
  }
  return 0;
}
