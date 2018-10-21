#ifndef UTIL_H
#define UTIL_H

char *trim_space_left(char *string);
char *trim_space_right(char *string);
char *trim_space(char *string);

enum ConnectType { CONNECT_NONE, CONNECT_POSITIVE, CONNECT_PASSIVE };

#endif