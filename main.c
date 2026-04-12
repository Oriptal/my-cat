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

int main(int argc, char **argv) {
  const char *WRONG_ARGUMENTS = "Неправильное количество аргументов!\n";
  const char *FILE_NOT_FOUND = "Не удалось открыть файл.";
  const char *FILE_IO_ERROR = "Ошибка чтения файла.";
  const char *IO_ERROR = "Ошибка при выводе содержимого файла.";
  if (argc != 2) {
    size_t number_of_bytes = strlen(WRONG_ARGUMENTS);
    if (safety_write(STDERR_FILENO, WRONG_ARGUMENTS, number_of_bytes) == -1) {
      perror(IO_ERROR);
      return IO_ERROR_CODE;
    }
    return INCORRECT_AMOUNT_OF_ARGS;
  }
  const int file_descriptor = open(argv[1], O_RDONLY);
  if (file_descriptor == -1) {
    perror(FILE_NOT_FOUND);
    return FILE_OPEN_ERROR;
  }
  char buffer[NUMBER_OF_BYTES];
  ssize_t read_code = 0;
  while ((read_code = read(file_descriptor, buffer, NUMBER_OF_BYTES)) > 0) {
    if (safety_write(STDOUT_FILENO, buffer, read_code) == -1) {
      perror(IO_ERROR);
      close(file_descriptor);
      return IO_ERROR_CODE;
    }
  }
  if (read_code == -1) {
    perror(FILE_IO_ERROR);
    close(file_descriptor);
    return FILE_IO_ERROR_CODE;
  }

  close(file_descriptor);
  return 0;
}
