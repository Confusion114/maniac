#include "maniac/maniac.h"
#include "maniac/accuracy.h"
#include "maniac/panic.h"
#include <random>
#include <chrono>
#include <deque>

namespace maniac {

static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
static RandomWalkDrift drift;
static double last_error = 0.0;
static std::deque<double> recent_note_times;

double generate_timing_delta(const config::AccuracyParams& acc_params, const config::RandomWalkParams& rw_params) {
    double now = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count() * 1000.0;
    recent_note_times.push_back(now);
    while (!recent_note_times.empty() && recent_note_times.front() < now - 2000.0)
        recent_note_times.pop_front();
    double density = std::min(recent_note_times.size() / 2.0, 30.0) / 30.0;

    double raw_offset = 0.0;
    if (acc_params.enabled) {
        raw_offset = AccuracyMultiStddev::sample(rng, density, acc_params);
    } else {
        // Fallback to simple randomization (you can keep original logic here if needed)
        std::normal_distribution<> nd(0.0, 10.0);
        raw_offset = nd(rng);
    }

    const double correction_factor = 0.15;
    double corrected = raw_offset - (correction_factor * last_error);

    double final_offset = corrected;
    if (rw_params.enabled) {
        final_offset += drift.update(rng, rw_params);
    }

    last_error = final_offset;
    // Apply global compensation (can be moved outside)
    // final_offset += config::get().compensation_offset; // you'll wire this later
    if (final_offset > 50.0) final_offset = 50.0;
    if (final_offset < -50.0) final_offset = -50.0;
    return final_offset;
}

} // namespace maniac
