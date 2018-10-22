#include "server.h"

char rootDir[20];
int portNum;

int main(int argc, char *argv[]) {
  srand((unsigned)time(NULL));
  int parse_val = parse_commandline(argc, argv);
  if (parse_val == -1) {
    printf("Parse command line failed\n");
    return 1;
  } else if (parse_val == 1)
    return 0;
  int fd, struct_len, numbytes;
  struct sockaddr_in server_address;
  void *return_val;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(portNum);
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

int parse_commandline(int argc, char *argv[]) {
  strcpy(rootDir, "/tmp");
  portNum = 21;
  const char *optstring = "p::r::h::";
  struct option opts[] = {
      {"root", 1, NULL, 'r'},
      {"port", 1, NULL, 'p'},
      {"help", 0, NULL, 'h'},
  };
  int c;
  while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
    switch (c) {
      case 'r': {
        DIR *dir = opendir(optarg);
        if (dir)
          closedir(dir);
        else {
          perror("Directory not exist");
          return -1;
        }
        printf("Root directory set to %s\n", optarg);
        strcpy(rootDir, optarg);
      } break;
      case 'p':
        portNum = atoi(optarg);
        if (!portNum) {
          printf("Invalid port\n");
          return -1;
        }
        printf("Port set to %d\n", portNum);
        break;
      case 'h':
        printf("Usage:\n");
        printf("--port / -p   Specify port number of the server\n");
        printf("--root / -r   Specify root directory of the server\n");
        printf("--help / -h   Print this help information\n");
        return 1;
      case '?':
        return -1;
      default:
        break;
    }
  }
  return 0;
}