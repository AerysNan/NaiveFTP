#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>
#include <string.h>

#define CODE_LENGTH 3
#define DIR_LENGTH 500
#define PORT_ARGUMENT 6

enum LoginStatus { LOG_OUT, LOG_PASS, LOG_IN };
enum RenameStatus { RENAME_NONE, RENAME_PROGRESS };
enum ConnectType { CONNECT_NONE, CONNECT_POSITIVE, CONNECT_PASSIVE };

struct User {
  char username[25];
  char password[25];
};

struct Status {
  char userName[25];
  char rnfName[25];
  char rootDir[20];
  char currentDir[100];
  int clientIP[4];
  int clientPort[2];
  int serverIP[4];
  int serverPort[2];
  int fd_transport;
  int fd_command;
  int bytesSent;
  int bytesReceived;
  int restartPos;
  enum LoginStatus loginStatus;
  enum RenameStatus renameStatus;
  enum ConnectType connectType;
};

struct Command {
  char text[5];
  int (*handler)(char *request, char *response, struct Status *status);
};

struct PathNode {
  char dir[50];
  struct PathNode *prev;
  struct PathNode *next;
};

char *trim_space_left(char *string);
char *trim_space_right(char *string);
char *trim_space(char *string);
void toLower(char *string);

int path_join(char *path, struct Status *status, char *out);
int path_squash(char *path, char *squashed);

#endif