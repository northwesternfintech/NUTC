#include "tick_manager.hpp"

#include <numeric>

namespace nutc {
namespace ticks {

auto
TickManager::notify_tick_() // NOLINT
{
    auto start = std::chrono::high_resolution_clock::now();
    for (TickObserver* observer : first_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : second_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : third_observers_) {
        observer->on_tick(current_tick_);
    }

    for (TickObserver* observer : fourth_observers_) {
        observer->on_tick(current_tick_);
    }
    auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

TickManager::tick_metrics_t TickManager::get_tick_metrics() const {
  std::priority_queue<milliseconds, std::vector<milliseconds>> temp_queue;
  for(const auto& tick_time : last_1000_tick_times_) {
    temp_queue.push(tick_time);
  }

  milliseconds median_tick_ms = milliseconds(0);
  milliseconds top_1p_ms = milliseconds(0);
  milliseconds top_5p_ms = milliseconds(0);
  milliseconds top_10p_ms = milliseconds(0);
  milliseconds top_50p_ms = milliseconds(0);
  milliseconds total_ms = milliseconds(0);

  size_t num_ticks = last_1000_tick_times_.size();

  if(num_ticks < 100) {
    return {milliseconds(0), milliseconds(0), milliseconds(0), milliseconds(0), milliseconds(0)};
  }

  for(size_t i = 0; i < num_ticks/100; i++) {
    top_1p_ms += temp_queue.top();
    top_5p_ms += temp_queue.top();
    top_10p_ms += temp_queue.top();
    top_50p_ms += temp_queue.top();
    total_ms += temp_queue.top();
    temp_queue.pop();
  }
  
  for(size_t i = num_ticks/100; i < num_ticks/20; i++) {
    top_5p_ms += temp_queue.top();
    top_10p_ms += temp_queue.top();
    top_50p_ms += temp_queue.top();
    total_ms += temp_queue.top();
    temp_queue.pop();
  }
  
  for(size_t i = num_ticks/20; i < num_ticks/10; i++) {
    top_10p_ms += temp_queue.top();
    top_50p_ms += temp_queue.top();
    total_ms += temp_queue.top();
    temp_queue.pop();
  }
  
  for(size_t i = num_ticks/10; i < num_ticks/2; i++) {
    top_50p_ms += temp_queue.top();
    total_ms += temp_queue.top();
    temp_queue.pop();
  }
  median_tick_ms = temp_queue.top();

  for(size_t i = num_ticks/2; i < num_ticks; i++) {
    total_ms += temp_queue.top();
    temp_queue.pop();
  }

  return {100*top_1p_ms/num_ticks, 20*top_5p_ms/num_ticks, 10*top_10p_ms/num_ticks, 2*top_50p_ms/num_ticks, median_tick_ms, total_ms/num_ticks};
}

void
TickManager::run_()
{
    using namespace std::chrono;
    auto next_tick = steady_clock::now();

    while (running_) {
        next_tick += delay_time_;
        std::this_thread::sleep_until(next_tick);
        current_tick_++;
        auto time = notify_tick_();
        last_1000_tick_times_.push_front(duration_cast<milliseconds>(time));
        if(last_1000_tick_times_.size() > 1000) {
          last_1000_tick_times_.pop_back();
        }
    }
}

} // namespace ticks
} // namespace nutc
