#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "database.h"
#define KEY_LENGTH 16
#define RANDOM_SEED 0xdeadbeef

static void *thread(void *args);
static void interrupt_handler(int sig);

static pthread_t *threads;
static char **keys;
static int nthreads;
static int npairs;
static int running;

int main(int argc, char **argv) {
  struct timeval tv;
  int i, j;

  srand48(RANDOM_SEED);
  signal(SIGINT, interrupt_handler);

  if (argc < 3) {
    fprintf(stderr, "Usage: %s nthreads npair\n", argv[0]);
    return 1;
  }

  nthreads = atoi(argv[1]);
  if (nthreads < 1) {
    fprintf(stderr, "Error: Number of threads should be at least 1\n");
    return 1;
  }

  npairs = atoi(argv[2]);
  if (npairs < 1) {
    fprintf(stderr, "Error: At least 1 key-value pair is required.\n");
    return NETLINK_INIT_ERROR;
  }

  if (client_init()) {
    fprintf(stderr, "Failed to initialize netlink.\n");
    return 1;
  }

  /* Generate keys to use (random seed is 0xdeadbeef). */
  printf("Generating keys...\n");
  keys = (char **) malloc(npairs * sizeof(char *));
  for (i = 0; i < npairs; i++) {
    keys[i] = (char *) calloc(KEY_LENGTH + 1, 1);
    for (j = 0; j < KEY_LENGTH; j++)
      keys[i][j] = (char) (drand48() * 26 + 97);
    printf("  %s\n", keys[i]);
  }

  /* Need a litle better seed for values. */
  gettimeofday(&tv, NULL);
  srand48(tv.tv_usec);

  running = 1;
  threads = (pthread_t *) malloc(nthreads * sizeof(pthread_t));
  for (i = 0; i < nthreads; i++)
    pthread_create(threads + i, NULL, thread, NULL);

  printf("Running...\n");
  for (i = 0; i < nthreads; i++)
    pthread_join(threads[i], NULL);
  free(threads);

  for (i = 0; i < npairs; i++)
    free(keys[i]);
  free(keys);
  printf("Done\n");
  return 0;
}

void *thread(void *argv) {
  unsigned short xsubi[3];
  int op, index, value = 0;
  size_t length;

  /* Local copy of the database. */
  database_t db = database_new();

  while (running) {
    op = (int) (2 * erand48(xsubi));
    index = (int) (npairs * erand48(xsubi));

    /* get or set value? */
    switch (op) {
    case 0:

      /* Set a value. */
      value = (int) (1000000 * erand48(xsubi));
      database_set(db, keys[index], (char *) value, sizeof(char *));
      client_set(keys[index], (char *) &value, sizeof(char *));
      break;
    default:

      /* Get a value. */
      client_get(keys[index], (char **) &value, &length);
      database_set(db, keys[index], (char *) value, sizeof(char *));
      break;
    }
  }

  database_free(db);
  return NULL;
}

void interrupt_handler(int sig) {
  running = 0;
}
