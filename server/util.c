#include "util.h"

struct Status;

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

int path_join(char *path, struct Status *status, char *out) {
  strcpy(out, status->rootDir);
  char input[DIR_LENGTH];
  memset(input, 0, DIR_LENGTH);
  if (*path == 0) {
    strcat(out, status->currentDir);
    return 0;
  }
  if (path[0] == '/')
    strcat(input, path);
  else {
    strcat(input, status->currentDir);
    strcat(input, "/");
    strcat(input, path);
  }
  char squashed[DIR_LENGTH];
  memset(squashed, 0, DIR_LENGTH);
  int retVal = path_squash(input, squashed);
  strcat(out, squashed);
  return retVal;
}

int path_squash(char *path, char *squashed) {
  if (*path == 0) return 0;
  char *p = path;
  int abs_path = (*p == '/') ? 1 : 0;
  char *end = path + strlen(path);
  struct PathNode *head = (struct PathNode *)malloc(sizeof(struct PathNode));
  struct PathNode *tail = head;
  head->next = NULL;
  head->prev = NULL;
  while (p < end) {
    if (*p == '/')
      p++;
    else {
      int i = 0;
      while (*(p + i) != '/' && p + i < end) i++;
      char dir[50];
      strncpy(dir, p, i);
      dir[i] = '\0';
      p += i;
      if (strcmp(dir, ".") == 0)
        continue;
      else if (strcmp(dir, "..") == 0) {
        if (head == tail) return -1;
        tail = tail->prev;
        free(tail->next);
        tail->next = NULL;
      } else {
        struct PathNode *node = (struct PathNode *)malloc(sizeof(struct PathNode));
        strcpy(node->dir, dir);
        tail->next = node;
        node->prev = tail;
        tail = node;
        tail->next = NULL;
      }
    }
  }
  if (abs_path) strcpy(squashed, "/");
  struct PathNode *node = head->next;
  while (node) {
    strcat(squashed, node->dir);
    if (node->next) strcat(squashed, "/");
    node = node->next;
  }
  node = head;
  while (node) {
    struct PathNode *t = node;
    node = node->next;
    free(t);
  }
  return 0;
}
