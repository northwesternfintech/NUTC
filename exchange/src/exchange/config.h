#pragma once

// TODO: a lot of these should be in a config file, not here

#define VERSION          "1.0"
#define STARTING_CAPITAL 100000
#define DEBUG_NUM_USERS  1

#define CLIENT_WAIT_SECS 3
#define ORDER_EXPIRATION_TIME 10

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

enum class Mode { SANDBOX, DEV, NORMAL, BOTS_ONLY };

#define RETAIL_ORDER_OFFSET .02 // How much retail orders are offset from theo price. Ex. .02 means buy order is theo+.02 to ensure it gets filled
#define RETAIL_ORDER_SIZE .05 // How much of the interest limit to use for retail orders
