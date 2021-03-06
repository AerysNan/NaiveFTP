#include "server.h"

extern struct User userList[30];

struct Command commandList[] = {
    {"user", handler_user},
    {"pass", handler_pass},
    {"retr", handler_retr},
    {"stor", handler_stor},
    {"quit", handler_quit},
    {"syst", handler_syst},
    {"type", handler_type},
    {"port", handler_port},
    {"pasv", handler_pasv},
    {"list", handler_list},
    {"rnfr", handler_rnfr},
    {"rnto", handler_rnto},
    {"mkd", handler_mkd},
    {"cwd", handler_cwd},
    {"pwd", handler_pwd},
    {"rmd", handler_rmd},
    {"dele", handler_dele},
    {"abor", handler_abor},
    {"rest", handler_rest},
};

int handler_request(char *request, char *response, struct Status *status)
{
  int size = sizeof(commandList) / sizeof(*commandList);
  command_tolower(request);
  for (int i = 0; i < size; i++)
  {
    struct Command *command = &commandList[i];
    int len = strlen(command->text);
    if (strncmp(request, command->text, len) != 0)
      continue;
    return command->handler(trim_space(request + len), response, status);
  }
  return handler_response(504, "Command not implemented\n", response, status);
}

int handler_response(int code, char *description, char *response, struct Status *status)
{
  sprintf(response, "%d ", code);
  char *begin = response + CODE_LENGTH + 1;
  strcpy(begin, description);
  *(begin + strlen(description)) = '\0';
  send(status->fd_command, response, strlen(response), 0);
  if (code == 221)
    return -1;
  return 0;
}

int handler_user(char *request, char *response, struct Status *status)
{
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  if (status->loginStatus == LOG_IN)
    return handler_response(530, "Already logged in\n", response, status);
  status->loginStatus = LOG_PASS;
  strcpy(status->userName, request);
  return handler_response(331, "User name okay, need password\n", response, status);
}

int handler_pass(char *request, char *response, struct Status *status)
{
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  if (status->loginStatus == LOG_OUT)
    return handler_response(503, "Login with USER first\n", response, status);
  if (status->loginStatus == LOG_IN)
    return handler_response(202, "Already logged in\n", response, status);
  if (strcmp(status->userName, "anonymous") == 0)
  {
    status->loginStatus = LOG_IN;
    return handler_response(230, "OK. Current restricted directory is /\n", response, status);
  }
  int size = sizeof(userList) / sizeof(*userList);
  for (int i = 0; i < size; i++)
  {
    if (strcmp(userList[i].username, status->userName) == 0)
    {
      if (strcmp(userList[i].password, request) == 0)
      {
        status->loginStatus = LOG_IN;
        return handler_response(230, "User logged in, proceed\n", response, status);
      }
    }
  }
  status->loginStatus = LOG_OUT;
  return handler_response(530, "Authentication failed\n", response, status);
}

int handler_quit(char *request, char *response, struct Status *status)
{
  status->loginStatus = LOG_OUT;
  char message[BUFSIZ];
  sprintf(message, "Logout, %d bytes sent and %d bytes received\n", status->bytesReceived, status->bytesSent);
  return handler_response(221, message, response, status);
}

int handler_abor(char *request, char *response, struct Status *status)
{
  status->loginStatus = LOG_OUT;
  char message[BUFSIZ];
  sprintf(message, "Logout, %d bytes sent and %d bytes received\n", status->bytesReceived, status->bytesSent);
  return handler_response(221, message, response, status);
}

int handler_rest(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  int length = atoi(request);
  status->restartPos = length;
  char message[50];
  sprintf(message, "Restarting at %d\n", length);
  return handler_response(350, message, response, status);
}

int handler_syst(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request != 0)
    return handler_response(501, "Syntax error\n", response, status);
  return handler_response(215, "UNIX Type: L8\n", response, status);
}

int handler_type(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (strcmp(request, "I") == 0)
    return handler_response(200, "Type set to I.\n", response, status);
  return handler_response(504, "Unknown type\n", response, status);
}

int handler_port(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (status->connectType != CONNECT_NONE)
  {
    close(status->fd_transport);
    status->connectType = CONNECT_NONE;
  }
  int h[4], p0, p1;
  int match_count = sscanf(request, "%d,%d,%d,%d,%d,%d", &h[0], &h[1], &h[2], &h[3], &p0, &p1);
  if (match_count != PORT_ARGUMENT)
    return handler_response(501, "Syntax error\n", response, status);
  for (int i = 0; i < 4; i++)
    status->clientIP[i] = h[i];
  status->clientPort[0] = p0;
  status->clientPort[1] = p1;
  if ((status->fd_transport = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return handler_response(425, "Create socket failed\n", response, status);
  status->connectType = CONNECT_POSITIVE;
  return handler_response(200, "Command PORT okay\n", response, status);
}

int handler_pasv(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request != 0)
    return handler_response(501, "Syntax error\n", response, status);
  if (status->connectType != CONNECT_NONE)
  {
    close(status->fd_transport);
    status->connectType = CONNECT_NONE;
  }
  struct sockaddr_in serverAddress;
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  bzero(&(serverAddress.sin_zero), 8);
  if ((status->fd_transport = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return handler_response(425, "Create socket failed\n", response, status);
  status->connectType = CONNECT_PASSIVE;
  int struct_len = sizeof(struct sockaddr_in);
  while (1)
  {
    uint16_t port = (uint16_t)(rand() % 45535 + 20000);
    serverAddress.sin_port = htons(port);
    if (bind(status->fd_transport, (struct sockaddr *)&serverAddress, struct_len) == -1)
      continue;
    if (listen(status->fd_transport, 1) == -1)
      continue;
    status->serverPort[0] = (int)(port >> 8);
    status->serverPort[1] = (int)(port & 0xff);
    char message[100];
    sprintf(message, "Entering Passive Mode(%d,%d,%d,%d,%d,%d)\n", status->serverIP[0], status->serverIP[1], status->serverIP[2], status->serverIP[3], (port >> 8), (port & 0xff));
    return handler_response(227, message, response, status);
  }
}

int handler_retr(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  int retVal;
  if (status->connectType == CONNECT_NONE)
    retVal = handler_response(425, "Cannot open data connection\n", response, status);
  else if (status->connectType == CONNECT_POSITIVE)
    retVal = retr_port(path, response, status);
  else if (status->connectType == CONNECT_PASSIVE)
    retVal = retr_pasv(path, response, status);
  status->connectType = CONNECT_NONE;
  return retVal;
}

int handler_stor(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  int retVal;
  if (status->connectType == CONNECT_NONE)
    retVal = handler_response(425, "Cannot open data connection\n", response, status);
  else if (status->connectType == CONNECT_POSITIVE)
    retVal = stor_port(path, response, status);
  else if (status->connectType == CONNECT_PASSIVE)
    retVal = stor_pasv(path, response, status);
  status->connectType = CONNECT_NONE;
  return retVal;
}

int handler_list(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  int retVal;
  if (status->connectType == CONNECT_NONE)
    retVal = handler_response(425, "Cannot open data connection\n", response, status);
  else if (status->connectType == CONNECT_POSITIVE)
    retVal = list_port(path, response, status);
  else if (status->connectType == CONNECT_PASSIVE)
    retVal = list_pasv(path, response, status);
  status->connectType = CONNECT_NONE;
  return retVal;
}

int handler_rnfr(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  if (access(path, W_OK) == 0)
  {
    status->renameStatus = RENAME_PROGRESS;
    strcpy(status->rnfName, path);
    return handler_response(350, "File exists, ready for destination\n", response, status);
  }
  else
    return handler_response(550, "File not exist\n", response, status);
}

int handler_rnto(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  if (status->renameStatus != RENAME_PROGRESS)
    return handler_response(503, "File not specified\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  status->renameStatus = RENAME_NONE;
  if (rename(status->rnfName, path) == 0)
    return handler_response(250, "File successfully renamed or moved\n", response, status);
  return handler_response(553, "Cannot rename or move file\n", response, status);
}

int handler_mkd(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  if (mkdir(path, 0777) == 0)
    return handler_response(250, "Directory successfully created\n", response, status);
  return handler_response(550, "Create directory failed\n", response, status);
}

int handler_cwd(char *request, char *response, struct Status *status)
{
  if (status->loginStatus != LOG_IN)
    return handler_response(530, "User not logged in\n", response, status);
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  DIR *dir = opendir(path);
  if (!dir)
    return handler_response(550, "No such directory\n", response, status);
  closedir(dir);
  strcpy(status->currentDir, path + strlen(status->rootDir));
  char message[200];
  strcpy(message, "Current working directory successfully changed to ");
  if (status->currentDir)
    strcat(message, status->currentDir);
  else
    strcat(message, "/");
  strcat(message, "\n");
  return handler_response(250, message, response, status);
}

int handler_pwd(char *request, char *response, struct Status *status)
{
  char message[DIR_LENGTH];
  strcpy(message, "Current working directory: ");
  if (*status->currentDir == 0)
    strcat(message, "/");
  else
    strcat(message, status->currentDir);
  return handler_response(257, message, response, status);
}

int handler_rmd(char *request, char *response, struct Status *status)
{
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  if (rmdir(path) == 0)
    return handler_response(250, "Directory removed\n", response, status);
  return handler_response(550, "Cannot remove directory\n", response, status);
}

int handler_dele(char *request, char *response, struct Status *status)
{
  if (*request == 0)
    return handler_response(501, "Syntax error\n", response, status);
  char path[DIR_LENGTH];
  if (path_join(request, status, path) == -1)
    return handler_response(425, "Illegal path\n", response, status);
  if (remove(path) == 0)
    return handler_response(250, "File removed\n", response, status);
  return handler_response(550, "Cannot remove file\n", response, status);
}
