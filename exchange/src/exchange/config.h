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

// Brownian motion stuff
#define BROWNIAN_MOTION_MEAN_SIZE_EVENT  15
#define BROWNIAN_MOTION_STDEV_EVENT_SIZE 5
#define BROWNIAN_MOTION_DEVIATION        0.1
#define SKEW_SCALE                       20000 // Factor by which the market wants to return to baseline (larger = less)
#define SKEW_FACTOR                      4 // How much more market events skew than normal
