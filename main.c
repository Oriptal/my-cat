#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const int IO_ERROR_CODE = 0b1;
const int FILE_IO_ERROR_CODE = 0b10;
const int FILE_OPEN_ERROR = 0b100;
const int UNKNOWN_FLAG = 0b1000;
const int MALLOC_EXCEPTION = 0b10000;
const int NUMBER_OF_BYTES = 1 << 12;

const char *IO_ERROR = "Ошибка при выводе содержимого файла.";
const char *FILE_IO_ERROR = "Ошибка чтения файла.";
const char *INVALID_OPTION = "Некорректный флаг.";

int safety_write(const int fd, const char *str, size_t n) {
  ssize_t total_written = 0;
  while (total_written < (ssize_t)n) {
    size_t len = n - total_written;
    ssize_t written = write(fd, str + total_written, len);
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
    case 'b':
      *mask |= 1 << 1;
      break;
    case 's':
      *mask |= 1 << 2;
      break;
    case 'E':
      *mask |= 1 << 3;
      break;
    default:
      return args[i];
      break;
    }
  }
  return 0;
}

ssize_t find_delim(const char *str, size_t start, size_t read_code,
                   char delim) {
  size_t ind = start;
  while (ind < read_code && str[ind] != delim) {
    ind++;
  }
  return ind;
}

int print_line_count(const int line_count) {
  char line_buffer[10];
  snprintf(line_buffer, 10, "%6d\t", line_count);
  if (safety_write(STDOUT_FILENO, line_buffer, strlen(line_buffer)) ==
      IO_ERROR_CODE) {
    perror(IO_ERROR);
    return IO_ERROR_CODE;
  }
  return 0;
}

int print_char(char ch) {
  if (safety_write(STDOUT_FILENO, &ch, 1) == IO_ERROR_CODE) {
    perror(IO_ERROR);
    return IO_ERROR_CODE;
  }
  return 0;
}

int cat_fd(const int fd, int *line_count, const int mask) {
  char buffer[NUMBER_OF_BYTES];
  ssize_t read_code = 0;
  while ((read_code = read(fd, buffer, NUMBER_OF_BYTES)) > 0) {
    ssize_t index = -1;
    bool prev_was_blank = false;
    while (index < read_code) {
      index++;
      ssize_t old_index = index;
      index = find_delim(buffer, index, read_code, '\n');
      bool is_blank_line = (old_index == index);
      if (mask & (1 << 2) && is_blank_line && prev_was_blank) {
        continue;
      }
      size_t len = index - old_index;
      if (mask & (1 << 3)) {
      }
      if (!(mask & (1 << 1) && is_blank_line)) {
        if (mask & ((1 << 0) | (1 << 1))) {
          if (print_line_count(*line_count) != 0) {
            return IO_ERROR_CODE;
          }
        }
      }

      if (safety_write(STDOUT_FILENO, buffer + old_index, len) ==
          IO_ERROR_CODE) {
        perror(IO_ERROR);
        return IO_ERROR_CODE;
      }

      if (buffer[old_index + len] == '\n') {
        if (mask & (1 << 3) && print_char('$') != 0) {
          return IO_ERROR_CODE;
        }
        if (print_char('\n') != 0) {
          return IO_ERROR_CODE;
        }
      }

      if (!(mask & (1 << 1) && index == old_index)) {
        (*line_count)++;
      }
      prev_was_blank = is_blank_line;
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
  int line_count = 1;
  if (argc == 1) {
    result |= cat_fd(STDIN_FILENO, &line_count, mask);
  }
  bool mandPath = false;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-") == 0) {
      result |= cat_fd(STDIN_FILENO, &line_count, mask);
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
      result |= cat_fd(fd, &line_count, mask);
      close(fd);
    }
  }
  return result;
}
