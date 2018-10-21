#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"

#define CODE_LENGTH 3
#define DIR_LENGTH 100
#define PORT_ARGUMENT 6

enum LoginStatus { LOG_OUT, LOG_PASS, LOG_IN };
enum RenameStatus { RENAME_NONE, RENAME_PROGRESS };

struct User {
  char username[25];
  char password[25];
};

struct Status {
  char userName[25];
  char rnfName[25];
  int clientIP[4];
  int clientPort[2];
  int serverIP[4];
  int serverPort[2];
  int fd_transport;
  int fd_command;
  enum LoginStatus loginStatus;
  enum RenameStatus renameStatus;
  enum ConnectType connectType;
};

struct Command {
  char text[5];
  int (*handler)(char *request, char *response, struct Status *status);
};

int handler_request(char *request, char *response, struct Status *status);
int handler_response(int code, char *description, char *response, struct Status *status);

int handler_user(char *request, char *response, struct Status *status);
int handler_pass(char *request, char *response, struct Status *status);
int handler_retr(char *request, char *response, struct Status *status);
int handler_stor(char *request, char *response, struct Status *status);
int handler_quit(char *request, char *response, struct Status *status);
int handler_syst(char *request, char *response, struct Status *status);
int handler_type(char *request, char *response, struct Status *status);
int handler_port(char *request, char *response, struct Status *status);
int handler_pasv(char *request, char *response, struct Status *status);
int handler_list(char *request, char *response, struct Status *status);
int handler_rnfr(char *request, char *response, struct Status *status);
int handler_rnto(char *request, char *response, struct Status *status);
int handler_mkd(char *request, char *response, struct Status *status);
int handler_cwd(char *request, char *response, struct Status *status);
int handler_pwd(char *request, char *response, struct Status *status);
int handler_rmd(char *request, char *response, struct Status *status);
int handler_dele(char *request, char *response, struct Status *status);

int new_connection(void *new_fd);

int list_port(char *request, char *response, struct Status *status);
int list_pasv(char *request, char *response, struct Status *status);

int send_data(int fd, FILE *pipe);

#endif