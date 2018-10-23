#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "util.h"

int parse_commandline(int argc, char *argv[]);
int parse_userlist();

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
int handler_dele(char *request, char *response, struct Status *status);
int handler_abor(char *request, char *response, struct Status *status);
int handler_rest(char *request, char *response, struct Status *status);
int handler_mkd(char *request, char *response, struct Status *status);
int handler_cwd(char *request, char *response, struct Status *status);
int handler_pwd(char *request, char *response, struct Status *status);
int handler_rmd(char *request, char *response, struct Status *status);

int new_connection(void *new_fd);

int list_port(char *request, char *response, struct Status *status);
int list_pasv(char *request, char *response, struct Status *status);
int stor_port(char *request, char *response, struct Status *status);
int stor_pasv(char *request, char *response, struct Status *status);
int retr_port(char *request, char *response, struct Status *status);
int retr_pasv(char *request, char *response, struct Status *status);

int send_data(int fd, FILE *pipe, struct Status *status);
int recv_data(int fd, FILE *pipe, struct Status *status);

#endif