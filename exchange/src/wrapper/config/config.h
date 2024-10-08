#pragma once

/***************************** Configuration *********************************/

// Version info

// Logging
#define LOG_BACKTRACE_SIZE 10

#define LOG_DIR            "logs"

#define LOG_FILE_SIZE      (1024 * 1024 / 2) // 512 KB
#define LOG_BACKUP_COUNT   5



/**
 * If we are in debug mode.
 *
 * Why is this a function macro?
 * It prevents silent undefined macro errors.
 *
 * https://stackoverflow.com/a/3160643
 */
// #ifdef NDEBUG
// #  define DEBUG() 1
// #else
// #  define DEBUG() 0
// #endif
