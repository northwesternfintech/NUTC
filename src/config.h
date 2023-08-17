// logging
#define LOG_BACKTRACE_SIZE 10

#define LOG_DIR            "logs"
#define LOG_FILE           (LOG_DIR "/app.log")

#define LOG_FILE_SIZE      (1024 * 1024 / 2) // 512 KB
#define LOG_BACKUP_COUNT   5

// firebase
//#define FIREBASE_URL "https://nutc-web-default-rtdb.firebaseio.com/"
#define FIREBASE_URL "http://127.0.0.1:4000/database/"
