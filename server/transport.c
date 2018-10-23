#include "server.h"

int list_port(char *path, char *response, struct Status *status) {
  struct sockaddr_in clientAddress;
  status->connectType = CONNECT_NONE;
  clientAddress.sin_family = AF_INET;
  clientAddress.sin_port = htons((status->clientPort[0] << 8) + status->clientPort[1]);
  char ip[20];
  sprintf(ip, "%d.%d.%d.%d", status->clientIP[0], status->clientIP[1], status->clientIP[2], status->clientIP[3]);
  clientAddress.sin_addr.s_addr = inet_addr((const char *)ip);
  char cmd[DIR_LENGTH];
  sprintf(cmd, "ls -l ");
  strcat(cmd, path);
  FILE *pipe = popen(cmd, "r");
  if (!pipe) {
    close(status->fd_transport);
    return handler_response(550, "File listing failed\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  if (connect(status->fd_transport, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0) {
    close(status->fd_transport);
    handler_response(425, "Connection to client failed\n", response, status);
  }
  int retVal = send_data(status->fd_transport, pipe, status);
  close(status->fd_transport);
  if (pclose(pipe) == -1) return handler_response(551, "No such directory\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "Closing data connection\n", response, status);
}

int list_pasv(char *path, char *response, struct Status *status) {
  int struct_len;
  struct sockaddr_in clientAddress;
  int new_fd = accept(status->fd_transport, (struct sockaddr *)&clientAddress, (socklen_t *)&struct_len);
  if (new_fd == -1) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(425, "Connection to client failed\n", response, status);
  }
  char cmd[DIR_LENGTH];
  sprintf(cmd, "ls -l ");
  strcat(cmd, path);
  FILE *pipe = popen(cmd, "r");
  if (!pipe) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(550, "File listing failed\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  int retVal = send_data(new_fd, pipe, status);
  close(status->fd_transport);
  close(new_fd);
  if (pclose(pipe) == -1) return handler_response(551, "No such directory\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "Closing data connection\n", response, status);
}

int stor_port(char *request, char *response, struct Status *status) {
  struct sockaddr_in clientAddress;
  status->connectType = CONNECT_NONE;
  clientAddress.sin_family = AF_INET;
  clientAddress.sin_port = htons((status->clientPort[0] << 8) + status->clientPort[1]);
  char ip[20];
  sprintf(ip, "%d.%d.%d.%d", status->clientIP[0], status->clientIP[1], status->clientIP[2], status->clientIP[3]);
  clientAddress.sin_addr.s_addr = inet_addr((const char *)ip);
  FILE *pipe = fopen(request, "w+");
  if (!pipe) {
    close(status->fd_transport);
    return handler_response(550, "Failed to open file\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  if (connect(status->fd_transport, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0) {
    close(status->fd_transport);
    handler_response(425, "Connection to client failed\n", response, status);
  }
  int retVal = recv_data(status->fd_transport, pipe, status);
  close(status->fd_transport);
  if (pclose(pipe) == -1) return handler_response(551, "No such file\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "File transfer complete\n", response, status);
}

int stor_pasv(char *request, char *response, struct Status *status) {
  int struct_len;
  struct sockaddr_in clientAddress;
  int new_fd = accept(status->fd_transport, (struct sockaddr *)&clientAddress, (socklen_t *)&struct_len);
  if (new_fd == -1) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(425, "Connection to client failed\n", response, status);
  }
  FILE *pipe = fopen(request, "w+");
  if (!pipe) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(550, "Failed to open file\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  int retVal = recv_data(new_fd, pipe, status);
  close(status->fd_transport);
  close(new_fd);
  if (pclose(pipe) == -1) return handler_response(551, "No such directory\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "File transfer complete\n", response, status);
}
int retr_port(char *request, char *response, struct Status *status) {
  struct sockaddr_in clientAddress;
  status->connectType = CONNECT_NONE;
  clientAddress.sin_family = AF_INET;
  clientAddress.sin_port = htons((status->clientPort[0] << 8) + status->clientPort[1]);
  char ip[20];
  sprintf(ip, "%d.%d.%d.%d", status->clientIP[0], status->clientIP[1], status->clientIP[2], status->clientIP[3]);
  clientAddress.sin_addr.s_addr = inet_addr((const char *)ip);
  FILE *pipe = fopen(request, "r");
  fseek(pipe, status->restartPos, SEEK_SET);
  status->restartPos = 0;
  if (!pipe) {
    close(status->fd_transport);
    return handler_response(550, "Failed to open file\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  if (connect(status->fd_transport, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) < 0) {
    close(status->fd_transport);
    handler_response(425, "Connection to client failed\n", response, status);
  }
  int retVal = send_data(status->fd_transport, pipe, status);
  close(status->fd_transport);
  if (pclose(pipe) == -1) return handler_response(551, "No such file\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "File transfer complete\n", response, status);
}
int retr_pasv(char *request, char *response, struct Status *status) {
  int struct_len;
  struct sockaddr_in clientAddress;
  int new_fd = accept(status->fd_transport, (struct sockaddr *)&clientAddress, (socklen_t *)&struct_len);
  if (new_fd == -1) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(425, "Connection to client failed\n", response, status);
  }
  FILE *pipe = fopen(request, "r");
  fseek(pipe, status->restartPos, SEEK_SET);
  status->restartPos = 0;
  if (!pipe) {
    close(status->fd_transport);
    close(new_fd);
    return handler_response(550, "Failed to open file\n", response, status);
  }
  handler_response(150, "Opening data connection\n", response, status);
  int retVal = send_data(new_fd, pipe, status);
  close(status->fd_transport);
  close(new_fd);
  if (pclose(pipe) == -1) return handler_response(551, "No such directory\n", response, status);
  if (retVal < 0) return handler_response(426, "Send data error\n", response, status);
  return handler_response(226, "File transfer complete\n", response, status);
}

int send_data(int fd, FILE *pipe, struct Status *status) {
  char buffer[BUFSIZ];
  while (1) {
    memset(buffer, 0, BUFSIZ);
    unsigned length = fread(buffer, sizeof(char), BUFSIZ, pipe);
    status->bytesSent += length;
    send(fd, buffer, length, 0);
    if (length < BUFSIZ && ferror(pipe))
      return -1;
    else if (length < BUFSIZ)
      return 0;
  }
}

int recv_data(int fd, FILE *pipe, struct Status *status) {
  char buffer[BUFSIZ];
  int len;
  while ((len = recv(fd, buffer, BUFSIZ, 0)) > 0) {
    fwrite(buffer, sizeof(char), (unsigned)len, pipe);
    status->bytesReceived += len;
  }
  return 0;
}