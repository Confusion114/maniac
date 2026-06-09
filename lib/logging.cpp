#include "include/maniac/logging.h"  // you can create this header or just use these functions
#include <fstream>
#include <chrono>
#include <iomanip>
#include <mutex>

namespace maniac {

static std::ofstream log_file;
static std::mutex log_mutex;

void init_logging(const std::string& path) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) log_file.close();
    log_file.open(path, std::ios::out | std::ios::trunc);
    if (log_file.is_open()) {
        log_file << "hit_object_time_ms,scheduled_press_ms,actual_press_ms,delta_ms,column,note_type\n";
    }
}

void log_hit(double hit_time, double scheduled_time, double actual_time, double delta, int column, const std::string& note_type) {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) {
        log_file << std::fixed << std::setprecision(3)
                 << hit_time << ","
                 << scheduled_time << ","
                 << actual_time << ","
                 << delta << ","
                 << column << ","
                 << note_type << "\n";
    }
}

void close_logging() {
    std::lock_guard<std::mutex> lock(log_mutex);
    if (log_file.is_open()) log_file.close();
}

} // namespace maniac
