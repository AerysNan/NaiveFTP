#include "server.h"

extern char rootDir[20];
extern int address[4];

int new_connection(void *new_fd)
{
  int numbytes;
  char input_buffer[BUFSIZ];
  char output_buffer[BUFSIZ];
  struct Status status;
  int fd = *(int *)new_fd;
  status.connectType = CONNECT_NONE;
  status.renameStatus = RENAME_NONE;
  status.loginStatus = LOG_OUT;
  status.fd_command = fd;
  status.bytesSent = 0;
  status.bytesReceived = 0;
  status.restartPos = 0;
  strcpy(status.rootDir, rootDir);
  memset(status.currentDir, 0, strlen(status.currentDir));
  if (address[0] != -1)
  {
    status.serverIP[0] = address[0];
    status.serverIP[1] = address[1];
    status.serverIP[2] = address[2];
    status.serverIP[3] = address[3];
  }
  else
  {
    struct ifaddrs *addrList;
    getifaddrs(&addrList);
    for (struct ifaddrs *p = addrList; p; p = p->ifa_next)
    {
      if (p->ifa_addr && p->ifa_addr->sa_family == AF_INET)
      {
        struct sockaddr_in *p_addr = (struct sockaddr_in *)p->ifa_addr;
        in_addr_t addr = p_addr->sin_addr.s_addr;
        status.serverIP[3] = (int)((addr >> 24) & 0xff);
        status.serverIP[2] = (int)((addr >> 16) & 0xff);
        status.serverIP[1] = (int)((addr >> 8) & 0xff);
        status.serverIP[0] = (int)((addr & 0xff));
        if (status.serverIP[0] != 127)
          break;
      }
    }
  }
  handler_response(220, "Naive FTP server ready. Current working directory is /\n", output_buffer, &status);
  while ((numbytes = recv(status.fd_command, input_buffer, BUFSIZ, 0)) > 0)
  {
    input_buffer[numbytes - 2] = '\0';
    if (handler_request(input_buffer, output_buffer, &status) == -1)
      break;
  }
  close(fd);
  return 0;
}