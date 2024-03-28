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

/** Brownian motion parameters **/
#ifndef uint
    typedef unsigned int uint; //  stupid hack
#endif
namespace BrownianMotionParameters {
    /**
     * `DEVIATION`
     * Under the brownian motion model, d(price)/d(tick) is a normal distribution
     * around some mean. That mean is determined mathematically: either zero, or
     * skewed towards zero using `TICK_SKEW_SCALE`. This variable is the stdev.
     * 
     * Higher means more change per tick for all ticks; lower means less. This affects
     * market events too, which have a stdev = `DEVIATION`. Non market events have a stdev
     * equal to, strangely, `DEVIATION` * `EVENT_DEVIATION_FACTOR`
     */
    constexpr double DEVIATION = 0.1;
    
    /**
     * `EVENT_SIZE_MEAN`: Average length, in ticks, of a market event
     * `EVENT_SIZE_STDEV`: Stdev length of a market event
     * `EVENT_DEVIATION_FACTOR`: Market events are themselves long chains of normal distributions
     *                           that are unsigned and multiplied by either + or -. The stdev
     *                           equals `DEVIATION` * `EVENT_DEVIATION_FACTOR`
     * 
     * It should be quite trivial how to manipulate the first two variables. The third requires more experimentation.
     * A higher deviation factor makes market events inherently more drastic than non market events. A smaller factor
     * means they are closer to "the status quo" and don't really do anything different.
     */
    constexpr uint EVENT_SIZE_MEAN = 15;
    constexpr uint EVENT_SIZE_STDEV = 5;
    constexpr uint EVENT_DEVIATION_FACTOR = 4;
    
    /**
     * `TICK_SKEW_SCALE`
     * Skew scale is a hacky solution to avoid long term divergence, as if we treat brownian motion
     * as a series of normal distributions, as we approach infinitely many terms, we just get more and
     * more variability. If we want to force the brownian motion to stay somewhat close to baseline, we
     * skew it.
     * 
     * The way this works is by directly affecting the mean of d(price)/d(tick). Mean is no longer zero,
     * but rather -(current value)/(tick skew scale) to drag it closer to baseline by a little bit.
     * 
     * Because this is inverse, higher means more able to diverge over time;
     * smaller means more constrained to be close to baseline.
     */
    constexpr uint TICK_SKEW_SCALE = 20000;
}


#define RETAIL_ORDER_OFFSET .02 // How much retail orders are offset from theo price. Ex. .02 means buy order is theo+.02 to ensure it gets filled
#define RETAIL_ORDER_SIZE .05 // How much of the interest limit to use for retail orders
