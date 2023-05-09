#include "utils.h"

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

PROGRAM_INFO *create_program_info(
    pid_t pid, char *name, struct timeval timestamp
) {
  PROGRAM_INFO *info = malloc(sizeof(PROGRAM_INFO));

  info->pid = pid;
  info->timestamp = timestamp;
  strcpy(info->name, name);  // NOLINT

  return info;
}

HEADER *create_header(REQUEST_TYPE type, size_t size) {
  HEADER *header = malloc(sizeof(HEADER));

  header->type = type;
  header->size = size;

  return header;
}

void make_fifo(char *fifo_name) {
  if (mkfifo(fifo_name, 0666) == -1) {
    perror("mkfifo");
    exit(EXIT_FAILURE);
  }
}

char *create_fifo(pid_t pid) {
  char *fifo_name = malloc(sizeof(char) * 64);
  sprintf(fifo_name, "tmp/%d.fifo", pid);  // NOLINT

  if (mkfifo(fifo_name, 0666) == -1) {
    perror("mkfifo");
    exit(EXIT_FAILURE);
  }

  return fifo_name;
}

void open_fifo(int *fd, char *fifo_name, int flags) {
  *fd = open(fifo_name, flags);

  if (*fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
}

void close_fifo(int fd) {
  if (close(fd) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}

ssize_t write_to_fd(int fd, void *info, size_t size, REQUEST_TYPE type) {
  HEADER *header = create_header(type, size);
  ssize_t written_bytes = write(fd, header, sizeof(HEADER));
  free(header);

  if (written_bytes == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  ssize_t info_written_bytes = 0;
  if (info) {
    info_written_bytes = write(fd, info, size);

    if (info_written_bytes == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }
  }

  return written_bytes + info_written_bytes;
}

REQUEST_TYPE read_from_fd(int fd, void *info) {
  HEADER header;
  int read_bytes = read(fd, &header, sizeof(HEADER));

  if (read_bytes == -1 || header.type == ERROR) {
    perror("read or server error");
    exit(EXIT_FAILURE);
  }

  info = malloc(header.size);
  read_bytes = read(fd, info, header.size);

  if (read_bytes == -1) {
    perror("read");
    exit(EXIT_FAILURE);
  }

  return header.type;
}

int open_file_by_path(char *path, int flags, mode_t mode) {
  int fd = open(path, flags, mode);

  if (fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  return fd;
}

ssize_t simple_write_to_fd(int fd, void *info, size_t size) {
  ssize_t written_bytes = write(fd, info, size);

  if (written_bytes == -1 || written_bytes != (ssize_t)size) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  return written_bytes;
}

char *strdup(const char *s) {
  char *ptr = malloc(strlen(s) + 1);
  if (ptr == NULL) return NULL;

  strcpy(ptr, s);  // NOLINT

  return ptr;
}

// Source: https://stackoverflow.com/questions/15846762/timeval-subtract-explanation
int timeval_subtract(
    struct timeval *result, struct timeval *x, struct timeval *y
) {
  struct timeval yy = *y;
  y = &yy;

  // Perform the carry for the later subtraction by updating y
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }

  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  // Return 1 if result is negative
  return x->tv_sec < y->tv_sec;
}

// function that parses an array of pids in the format PID-1234 to an array of just the numbers
int *parse_pids(char **pids, int N) {
  int *parsed_pids = malloc(sizeof(int) * N + 1);

  // the first pid is always the pid that identifies the client
  parsed_pids[0] = getpid();

  for (int i = 1; i < N; i++) {
    char *pid = strtok(pids[i], "PID-");
    int pin_n = atoi(pid);
    parsed_pids[i] = pin_n;
  }

  return parsed_pids;
}

void divide_files_per_fork(int num_files, int *num_forks, int *files_per_fork) {
  int max_files_per_fork = 5;
  *num_forks = (num_files + max_files_per_fork - 1) / max_files_per_fork;
  *files_per_fork = (num_files + *num_forks - 1) / *num_forks;
}

int retrieve_time_from_file(int fd) {
    int BUFFER_SIZE = 1024;
    char *buffer = malloc(BUFFER_SIZE);
    int time = -1;

    while (read(fd, buffer, BUFFER_SIZE) > 0) {
        // Search for "DURATION[ms]: " in the buffer
        char *duration_pos = strstr(buffer, "DURATION[ms]: ");
        if (duration_pos != NULL) {
            // Retrieve the duration from the line
            sscanf(duration_pos, "DURATION[ms]: %d", &time);
            break;
        }
    }
    free(buffer);

    return time;
}
