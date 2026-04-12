#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const int IO_ERROR_CODE = -1;
const int FILE_IO_ERROR_CODE = 100;
const int INCORRECT_AMOUNT_OF_ARGS = 138;
const int FILE_OPEN_ERROR = 123;
const int NUMBER_OF_BYTES = 1 << 12;

const char *FILE_NOT_FOUND = "Не удалось открыть файл.";
const char *FILE_IO_ERROR = "Ошибка чтения файла.";
const char *IO_ERROR = "Ошибка при выводе содержимого файла.";
const char *WRONG_ARGUMENTS = "Неправильное количество аргументов!\n";

int safety_write(const int fd, const char *str, size_t n) {
  ssize_t total_written = 0;
  while (total_written < n) {
    ssize_t written = write(fd, str + total_written, n - total_written);
    if (written <= 0) {
      return -1;
    }
    total_written += written;
  }
  return 0;
}

int cat_fd(int fd) {
  if (fd == -1) {
    perror(FILE_NOT_FOUND);
    return FILE_OPEN_ERROR;
  }
  char buffer[NUMBER_OF_BYTES];
  ssize_t read_code = 0;
  while ((read_code = read(fd, buffer, NUMBER_OF_BYTES)) > 0) {
    if (safety_write(STDOUT_FILENO, buffer, read_code) == -1) {
      perror(IO_ERROR);
      close(fd);
      return IO_ERROR_CODE;
    }
  }
  if (read_code == -1) {
    perror(FILE_IO_ERROR);
    close(fd);
    return FILE_IO_ERROR_CODE;
  }

  close(fd);
  return 0;
}

int main(int argc, char **argv) {
  if (argc == 1) { // Нет аргументов
    cat_fd(STDIN_FILENO);
  }
  for (int i = 1; i < argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    cat_fd(fd);
  }
}
