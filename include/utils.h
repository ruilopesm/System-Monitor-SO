#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAIN_FIFO_NAME "tmp/monitor.fifo"

typedef enum request_type {
  NEW,
  PIPELINE,
  STATUS,
  UPDATE,
  ERROR,
  DONE,
  OK
} REQUEST_TYPE;

typedef struct program_info {
  int pid;
  char name[50];
  suseconds_t timestamp;
} PROGRAM_INFO;

typedef struct header {
  REQUEST_TYPE type;
  size_t size;
} HEADER;

PROGRAM_INFO *create_program_info(int pid, char *name, suseconds_t timestamp);

HEADER *create_header(REQUEST_TYPE type, size_t size);

char *create_fifo(int pid);

void open_fifo(int *fd, char *fifo_name, int flags);

int write_to_fd(int fd, void *info, size_t size, REQUEST_TYPE type);

REQUEST_TYPE read_from_fd(int fd, void *info, size_t size);

char *strdup(const char *s);

int timeval_subtract(
    struct timeval *result, struct timeval *x, struct timeval *y
);

#endif  // UTILS_H
