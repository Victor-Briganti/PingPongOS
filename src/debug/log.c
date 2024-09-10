#include "debug/log.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static FILE *output = NULL;
static int color = LOG_COLOR_DISABLE;
static int level = LOG_DEBUG;

// Color code
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define BLACK "\033[1;30m"

const char *level_string[] = {"TRACE", "DEBUG", "INFO",
                              "WARN",  "ERROR", "FATAL"};

const char *level_color[] = {BLUE, MAGENTA, GREEN, YELLOW, RED, BLACK};

static void print_time() {
  time_t now = time(NULL);
  struct tm *local_time = localtime(&now);
  char time_str[100];
  (void)strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
  (void)fprintf(output, "%s ", time_str);
}

static void print_level(int level) {
  if (color == LOG_COLOR_ENABLE) {
    switch (level) {
    case LOG_TRACE:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_TRACE],
                    level_string[LOG_TRACE], RESET);
      return;
    case LOG_INFO:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_INFO],
                    level_string[LOG_INFO], RESET);
      return;
    case LOG_DEBUG:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_DEBUG],
                    level_string[LOG_DEBUG], RESET);
      return;
    case LOG_WARN:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_WARN],
                    level_string[LOG_WARN], RESET);
      return;
    case LOG_ERROR:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_ERROR],
                    level_string[LOG_ERROR], RESET);
      return;
    case LOG_FATAL:
      (void)fprintf(output, "[%s%s%s] ", level_color[LOG_FATAL],
                    level_string[LOG_FATAL], RESET);
      return;
    default:
      (void)fprintf(output, "ERROR ON LOGS");
      return;
    }
  }

  switch (level) {
  case LOG_TRACE:
    (void)(void)fprintf(output, "[%s] ", level_string[LOG_TRACE]);
    return;
  case LOG_INFO:
    (void)fprintf(output, "[%s] ", level_string[LOG_INFO]);
    return;
  case LOG_DEBUG:
    (void)fprintf(output, "[%s] ", level_string[LOG_DEBUG]);
    return;
  case LOG_WARN:
    (void)fprintf(output, "[%s] ", level_string[LOG_WARN]);
    return;
  case LOG_ERROR:
    (void)fprintf(output, "[%s] ", level_string[LOG_ERROR]);
    return;
  case LOG_FATAL:
    (void)fprintf(output, "[%s] ", level_string[LOG_FATAL]);
    return;
  default:
    (void)fprintf(output, "ERROR ON LOGS");
    return;
  }
}

static void logger(int log_level, const char *file, int line, const char *fmt,
                   va_list args) {
  print_time();
  print_level(log_level);
  (void)fprintf(output, "%s:%d ", file, line);

  (void)vfprintf(output, fmt, args);
  (void)fprintf(output, "\n");
}

void __logger(int log_level, const char *file, int line, const char *fmt, ...) {
  if (output == NULL) {
    output = stderr;
  }

  if (log_level > level) {
    va_list args;
    va_start(args, fmt);
    logger(log_level, file, line, fmt, args);
    va_end(args);
  }
}

void log_set(FILE *file, int enable_color, int log_level) {
  output = file;
  color = enable_color;
  level = log_level;
}