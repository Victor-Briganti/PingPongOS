/*
 * PingPongOS - PingPong Operating System
 * Filename: log.h
 * Description: Logging library for debugging the system
 *
 * Author: Victor Briganti
 * Date: 2024-09-09
 * License: BSD 2
 */

#ifndef PP_LOG_H
#define PP_LOG_H

#include <stdio.h>

/**
 * @brief Enumeration for log levels in the logging system.
 *
 * Each log level represents a severity level for logging messages. Lower
 * log levels include messages from higher levels. For example, a log level
 * of `LOG_WARN` will include messages from `LOG_ERROR` and `LOG_FATAL`.
 *
 * Log levels in ascending order of severity:
 * - `LOG_TRACE`: Most detailed, used for tracing execution and debugging.
 * - `LOG_DEBUG`: General debugging information, less detailed than TRACE.
 * - `LOG_INFO`: Informational messages about normal operation.
 * - `LOG_WARN`: Warnings about potential issues that are not errors.
 * - `LOG_ERROR`: Error messages indicating problems that need attention.
 * - `LOG_FATAL`: Critical errors that cause the application to terminate.
 */
enum {
  LOG_TRACE,
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL,
};

#define LOG_COLOR_ENABLE 1
#define LOG_COLOR_DISABLE 0

#ifdef DEBUG
void __logger(int log_level, const char *file, int line, const char *func,
              const char *fmt, ...);
#define log_trace(...)                                                         \
  __logger(LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_debug(...)                                                         \
  __logger(LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_info(...)                                                          \
  __logger(LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_warn(...)                                                          \
  __logger(LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_error(...)                                                         \
  __logger(LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_fatal(...)                                                         \
  __logger(LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Configures the logging system.
 *
 * This function allows you to set the output file for logging, enable or
 * disable color in logs, and specify the minimum log level for messages
 * to be recorded.
 *
 * @param file Pointer to the file where logs should be written. If `NULL`,
 *             logging will use the stderr. If something different than NULL is
 *             passed, the user is responsible to close it.
 * @param enable_color Integer flag to enable or disable colored log output. Use
 *              `COLOR_ENABLE` to enable colors and `COLOR_DISABLE` to
 *              disable them. If not specified the color will be disable.
 * @param log_level The minimum log level to record. Messages with a log level
 *                  equal to or higher than this level will be logged. If the
 *                  level is not enabled the LOG_DEBUG is going to be used
 */
void log_set(FILE *file, int enable_color, int log_level);

#else
#define log_set(...) ;
#define log_trace(...)
#define log_debug(...)
#define log_info(...)
#define log_warn(...)
#define log_error(...)
#define log_fatal(...)
#endif

#endif // PP_LOG_H