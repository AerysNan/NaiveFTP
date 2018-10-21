#include "util.h"
#include <string.h>

char *trim_space_left(char *string) {
  int len = strlen(string);
  for (int i = 0; i < len; i++)
    if (string[i] != ' ') return string + i;
  return "";
}

char *trim_space_right(char *string) {
  int len = strlen(string);
  for (int i = len - 1; i >= 0; i--) {
    if (string[i] != ' ') return string;
    string[i] = 0;
  }
  return "";
}

char *trim_space(char *string) { return trim_space_right(trim_space_left(string)); }