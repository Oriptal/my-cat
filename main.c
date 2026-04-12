#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const int IO_ERROR_CODE = 0b1;
const int FILE_IO_ERROR_CODE = 0b10;
const int FILE_OPEN_ERROR = 0b100;
const int NUMBER_OF_BYTES = 1 << 12;

const char *IO_ERROR = "Ошибка при выводе содержимого файла.";
const char *FILE_IO_ERROR = "Ошибка чтения файла.";

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

int cat_fd(int fd) {
  char buffer[NUMBER_OF_BYTES];
  ssize_t read_code = 0;
  while ((read_code = read(fd, buffer, NUMBER_OF_BYTES)) > 0) {
    if (safety_write(STDOUT_FILENO, buffer, read_code) == -1) {
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
  int result = 0;
  if (argc == 1) {
    result |= cat_fd(STDIN_FILENO);
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-") == 0) {
      result |= cat_fd(STDIN_FILENO);
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
