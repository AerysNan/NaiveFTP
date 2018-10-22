#include "server.h"

extern char rootDir[20];

int new_connection(void *new_fd) {
  int numbytes;
  char input_buffer[BUFSIZ];
  char output_buffer[BUFSIZ];
  struct Status status;
  status.connectType = CONNECT_NONE;
  status.renameStatus = RENAME_NONE;
  status.loginStatus = LOG_OUT;
  status.fd_command = *(int *)new_fd;
  strcpy(status.rootDir, rootDir);
  memset(status.currentDir, 0, strlen(status.currentDir));
  int tempfd = socket(AF_INET, SOCK_DGRAM, 0);
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, "enp0s8", IFNAMSIZ - 1);
  ioctl(tempfd, SIOCGIFADDR, &ifr);
  close(tempfd);
  sscanf(inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr), "%d.%d.%d.%d", &status.serverIP[0], &status.serverIP[1], &status.serverIP[2], &status.serverIP[3]);
  handler_response(220, "Naive FTP server ready. Current working directory is /\n", output_buffer, &status);
  while ((numbytes = recv(status.fd_command, input_buffer, BUFSIZ, 0)) > 0) {
    input_buffer[numbytes - 2] = '\0';
    if (handler_request(input_buffer, output_buffer, &status) == -1) break;
  }
  close(*(int *)new_fd);
  return 0;
}