#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const int IO_ERROR_CODE = 0b1;
const int FILE_IO_ERROR_CODE = 0b10;
const int FILE_OPEN_ERROR = 0b100;
const int UNKNOWN_FLAG = 0b1000;
const int NUMBER_OF_BYTES = 1 << 12;

const char *IO_ERROR = "Ошибка при выводе содержимого файла.";
const char *FILE_IO_ERROR = "Ошибка чтения файла.";
const char *INVALID_OPTION = "Некорректный флаг.";

int safety_write(const int fd, const char *str, size_t n) {
  ssize_t total_written = 0;
  while (total_written < n) {
    ssize_t written = write(fd, str + total_written, n - total_written);
    if (written <= 0) {
      return IO_ERROR_CODE;
    }
    total_written += written;
  }
  return 0;
}

int parse_short_opts(const char *args, int *mask) {
  size_t len = strlen(args);
  for (size_t i = 0; i < len; i++) {
    switch (args[i]) {
    case 'n':
      *mask |= 1 << 0;
      break;
    default:
      return args[i];
      break;
    }
  }
  return 0;
}

int cat_fd(int fd) {
  char buffer[NUMBER_OF_BYTES];
  ssize_t read_code = 0;
  while ((read_code = read(fd, buffer, NUMBER_OF_BYTES)) > 0) {
    if (safety_write(STDOUT_FILENO, buffer, read_code) == IO_ERROR_CODE) {
      perror(IO_ERROR);
      return IO_ERROR_CODE;
    }
  }
  if (read_code == -1) {
    perror(FILE_IO_ERROR);
    return FILE_IO_ERROR_CODE;
  }

  return 0;
}

int main(int argc, char **argv) {
  int mask = 0;
  int result = 0;
  if (argc == 1) {
    result |= cat_fd(STDIN_FILENO);
  }
  bool mandPath = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-") == 0) {
      result |= cat_fd(STDIN_FILENO);
      continue;
    }
    if (!mandPath && argv[i][0] == '-') { // флаг
      if (argv[i][1] == '-') {
        mandPath = true;
      } else {
        char *trimmed = argv[i] + 1;
        int parse_status = parse_short_opts(trimmed, &mask);
        if (parse_status != 0) {
          int write_status = safety_write(STDERR_FILENO, INVALID_OPTION,
                                          strlen(INVALID_OPTION));
          if (write_status != IO_ERROR_CODE) {
            perror(IO_ERROR);
          }
          return UNKNOWN_FLAG;
        }
      }
      continue;
    }
    int fd = open(argv[i], O_RDONLY);
    if (fd == -1) {
      perror(argv[i]);
      result |= FILE_OPEN_ERROR;
    } else {
      result |= cat_fd(fd);
      close(fd);
    }
  }
  return result;
}
