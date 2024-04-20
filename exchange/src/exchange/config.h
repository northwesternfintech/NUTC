#pragma once

#define DEBUG_NUM_USERS  10

// How many outgoing messages for one wrapper before we start dropping
#define MAX_OUTGOING_MQ_SIZE 1000

// Limit to 16kb
#define MAX_PIPE_MSG_SIZE 16000

// logging
#define LOG_BACKTRACE_SIZE 10

#define LOG_DIR            "logs"
#define LOG_FILE           (LOG_DIR "/app.log")
#define JSON_LOG_FILE      (LOG_DIR "/structured.log")

#define LOG_FILE_SIZE      (1024 * 1024 / 2) // 512 KB
#define LOG_BACKUP_COUNT   5

#define RABBITMQ_PORT 5672
// #define FIREBASE_URL "127.0.0.1:9000"
//
#define ALGO_DIR "algos"

// Brownian motion stuff
 #define BROWNIAN_MOTION_MEAN_SIZE_EVENT  10
 #define BROWNIAN_MOTION_STDEV_EVENT_SIZE 5
 #define BROWNIAN_MOTION_DEVIATION        0.2
 #define SKEW_SCALE                       20000 // Factor by which the market wants to return to baseline (larger = less)
 #define SKEW_FACTOR                      2 // How much more market events skew than normal


#define RETAIL_ORDER_OFFSET .02 // How much retail orders are offset from theo price. Ex. .02 means buy order is theo+.02 to ensure it gets filled
#define RETAIL_ORDER_SIZE .05 // How much of the interest limit to use for retail orders
