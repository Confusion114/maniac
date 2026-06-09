#pragma once
#include <deque>
#include "config.h"

namespace maniac {

bool is_panicking(const std::deque<double>& recent_times, double current_time, const config::PanicParams& params);
bool should_miss(std::mt19937& rng, const config::PanicParams& params);
bool should_extra_press(std::mt19937& rng, const config::PanicParams& params);

} // namespace maniac
