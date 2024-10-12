#pragma once

/***************************** Configuration *********************************/

// Version info
/* clang-format off */
/* NOLINTBEGIN */
#define VERSION_MAJOR @NUTC-client_VERSION_MAJOR@
#define VERSION_MINOR @NUTC-client_VERSION_MINOR@
#define VERSION_PATCH @NUTC-client_VERSION_PATCH@

#define NUTC_VERSION "@NUTC-client_VERSION@"
/* clang-format on */
/* NOLINTEND */

// Logging

#define LOG_FILE_SIZE      (1024 * 1024 / 2) // 512 KB
#define LOG_BACKUP_COUNT   5

#ifdef NUTC_LOCAL_DEV
#  define S3_URL        "http://localhost:4566"
#  define WEBSERVER_URL "http://localhost:16124"
#else
#  define S3_URL        "https://nutc.s3.us-east-2.amazonaws.com"
#  define WEBSERVER_URL "http://localhost:16124"
#endif

#define S3_BUCKET "nutc"

// Linting
#define LINT_AUTO_TIMEOUT_SECONDS 4

/**
 * If we are in debug mode.
 *
 * Why is this a function macro?
 * It prevents silent undefined macro errors.
 *
 * https://stackoverflow.com/a/3160643
 */
#ifdef NDEBUG
#  define DEBUG() 1
#else
#  define DEBUG() 0
#endif
