#include "server.h"

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));
  int fd, struct_len, numbytes;
  struct sockaddr_in server_address;
  void *return_val;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(21);
  server_address.sin_addr.s_addr = INADDR_ANY;
  bzero(&(server_address.sin_zero), 8);
  struct_len = sizeof(struct sockaddr_in);
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    perror("Create socket failed");
    return 1;
  }
  if (bind(fd, (struct sockaddr *)&server_address, struct_len) == -1) {
    perror("Bind to port failed");
    return 1;
  }
  if (listen(fd, 10) == -1) {
    perror("Listen failed");
    return 1;
  }
  while (1) {
    int new_fd;
    struct sockaddr_in client_address;
    new_fd = accept(fd, (struct sockaddr *)&client_address, &struct_len);
    if (new_fd == -1) {
      perror("Accept new connection failed");
      return 1;
    }
    pthread_t thread;
    if (pthread_create(&thread, NULL, (void *)&new_connection, (void *)&new_fd) != 0) {
      perror("Create new thread failed");
      return 1;
    }
  }
  close(fd);
  return 0;
}