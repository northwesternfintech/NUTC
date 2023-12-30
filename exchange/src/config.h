#pragma once

#define VERSION          "1.0"
#define STARTING_CAPITAL 100000
#define DEBUG_NUM_USERS  2

#define CLIENT_WAIT_SECS 3

// logging
#define LOG_BACKTRACE_SIZE 10

#define LOG_DIR            "logs"
#define LOG_FILE           (LOG_DIR "/app.log")
#define JSON_LOG_FILE      (LOG_DIR "/structured.log")

#define LOG_FILE_SIZE      (1024 * 1024 / 2) // 512 KB
#define LOG_BACKUP_COUNT   5

// firebase
#define FIREBASE_URL "https://nutc-web-default-rtdb.firebaseio.com/"

#define RABBITMQ_PORT 5672
// #define FIREBASE_URL "127.0.0.1:9000"
//
#define ALGO_DIR "algos"

enum class Mode { SANDBOX, DEV, PROD };
