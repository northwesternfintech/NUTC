#pragma once

#define DEBUG_NUM_USERS  2

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

//
#define ALGO_DIR "algos"



#define RETAIL_ORDER_OFFSET .02 // How much retail orders are offset from theo price. Ex. .02 means buy order is theo+.02 to ensure it gets filled
#define RETAIL_ORDER_SIZE .05 // How much of the interest limit to use for retail orders
