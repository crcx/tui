#define _POSIX_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>

extern char **environ;
int running = 1;

void dup_lim_exec(int fd, char *cmd, char *argv[]) {
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  dup2(fd, STDIN_FILENO);
  dup2(fd, STDOUT_FILENO);
  execv(cmd, argv);
}

void handle(int fd, char *cmd, char *argv[]) {
  pid_t pid = fork();
  if(pid != 0)
    return;
  else
    dup_lim_exec(fd, cmd, argv);
}

void sighandler(int s) {
  (void)s; /* To avoid a compiler warning. */
  running = 0;
}

void setup_signals(void) {
  int stopsignals[] = {SIGHUP, SIGINT, SIGQUIT, SIGPIPE, SIGALRM, SIGTERM, SIGUSR1, SIGUSR2};
  for(int i = 0; i < 8; i++)
    signal(stopsignals[i], sighandler);
  signal(SIGCHLD, SIG_IGN);
}

int getsocket(char *port) {
  struct addrinfo hints = {AI_PASSIVE, AF_UNSPEC, SOCK_STREAM, 0, 0, NULL, NULL, NULL};
  struct addrinfo *result;
  int enabled = 1;
  int connection;
  getaddrinfo(NULL, port, &hints, &result);
  connection = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  setsockopt(connection, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
  bind(connection, result->ai_addr, result->ai_addrlen);
  freeaddrinfo(result);
  listen(connection, SOMAXCONN);
  return connection;
}

int main(int argc, char *argv[]) {
  int incoming, client;
  if(argc < 3) {
    fprintf(stderr, "%s <port> command [ args ... ]\n", argv[0]);
  } else {
    incoming = getsocket(argv[1]);
    setup_signals();
    while(running) {
      client = accept(incoming, NULL, NULL);
      handle(client, argv[2], &argv[2]);
      close(client);
    }
    close(incoming);
  }
  return 0;
}
