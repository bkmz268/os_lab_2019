#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {

    int active_child_processes = 0;
    pid_t child_pid = fork();

    if (child_pid >= 0) {
        //successful fork()
         active_child_processes += 1;
      if (child_pid == 0) {
           // child process
          printf("active child prcesses count %i\n", active_child_processes);
          exit(0);
      }}

    return 0;
}