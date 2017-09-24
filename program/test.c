#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "conn.h"

int main(int argc, char **argv) {
  size_t message_length;
  size_t len1;
  size_t len2;
  size_t offset;
  char *data;

  char dummy = 0;

  len1 = strlen(argv[2]) + 1;
  len2 = (argc > 3) ? strlen(argv[3]) + 1: 1;

  message_length = len1 + 2 * sizeof(int) + 2;
  data = (char *) malloc(message_length);
  data[0] = 0;
  offset = 1;
  printf("|%d|", (int) data[0]);

  memcpy(data + offset, &len1, sizeof(int));
  printf("%d|", *(int *) (data + offset));
  offset += sizeof(int);

  memcpy(data + offset, &len2, sizeof(int));
  printf("%d|", *(int *) (data + offset));
  offset += sizeof(int);

  memcpy(data + offset, argv[2], len1);
  printf("%s|", data + offset);
  offset += len1;

  memcpy(data + offset, ((argc > 3) ? argv[3] : &dummy), len2);
  printf("%s|\n", data + offset);

  char buffer[4096];
  conn_t conn = conn_new();
  conn_send(conn, data, message_length);
  conn_recv(conn, buffer, 4096);
  conn_free(conn);
  free(data);
  return 0;
}
