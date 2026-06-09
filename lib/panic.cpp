#include "maniac/panic.h"
#include <random>

namespace maniac {

bool is_panicking(const std::deque<double>& recent_times, double current_time, const config::PanicParams& params) {
    if (!params.enabled) return false;
    int count = 0;
    double start = current_time - params.panic_window_ms;
    for (double t : recent_times) {
        if (t >= start && t <= current_time) count++;
    }
    if (count >= params.panic_threshold_notes) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        return dis(rng) < params.panic_chance;
    }
    return false;
}

bool should_miss(std::mt19937& rng, const config::PanicParams& params) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(rng) < params.miss_chance;
}

bool should_extra_press(std::mt19937& rng, const config::PanicParams& params) {
    std::uniform_real_distribution<> dis(0.0, 1.0);
    return dis(rng) < params.extra_press_chance;
}

} // namespace maniac
