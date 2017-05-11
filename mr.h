/*
 * file: mr.h
 * auth: guthrie linck (burpod@gmail.com)
 * date: 12/2/2016
 * desc: 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>

/*
 * reader_t
 */

typedef struct {
  char name[64];
  char file[128];
  char label[64];
  int active;
  int fd;
  pthread_t pt;
} reader_t;

void reader_thread(reader_t *r);
