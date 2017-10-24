#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "shared_map.h"

static shared_map_t map;

int main(int argc, char **argv) {
  struct stat st = {0};
  char *moddb_dir = "/var/tmp/shared_map";
  if (stat(moddb_dir, &st) == -1) {
    mkdir(moddb_dir, 0777);
  }

  map = shared_map_new(getpid());
  if (map == NULL) {
    fprintf(stderr, "Error: Failed to initialize Netlink\n");
    return 1;
  }

  int operations_count = 100000;
  long start, end;
  struct timeval timecheck;
  int i = 0;

  if (argc == 2) {
    if (!strcmp("r", argv[1])) {
      void *value;
      size_t length;
      int is_error_printed = 0;
      shared_map_insert(map, "test_key", "test_value", strlen("test_value") + 1);

      gettimeofday(&timecheck, NULL);
      start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

      for (i = 0; i < operations_count; i++) {
        if (shared_map_lookup(map, "test_key", &value, &length) && !is_error_printed) {
          printf("Key not found\n");
          is_error_printed = 1;
        }
      }

      gettimeofday(&timecheck, NULL);
      end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    } else if (!strcmp("w", argv[1])) {

      gettimeofday(&timecheck, NULL);
      start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

      for (i = 0; i < operations_count; i++) {
        shared_map_insert(map, "test_key", "test_value", strlen("test_value") + 1);
      }

      gettimeofday(&timecheck, NULL);
      end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    } else if (!strcmp("rw", argv[1])) {
      void *value;
      size_t length;
      int is_error_printed = 0;

      gettimeofday(&timecheck, NULL);
      start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;

      for (i = 0; i < operations_count; i++) {
        shared_map_insert(map, "test_key", "test_value", strlen("test_value") + 1);
        if (shared_map_lookup(map, "test_key", &value, &length) && !is_error_printed) {
          printf("Key not found\n");
          is_error_printed = 1;
        }
      }

      gettimeofday(&timecheck, NULL);
      end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000;
    }

    double time_seconds = (double)(end - start) / 1000.0;
    printf("%.3f\n", time_seconds);
  } else {
    printf("Wrong args, choose 'r', 'w' or 'rw'\n");
  }

  shared_map_free(map);
  return 0;
}
