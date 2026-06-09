#include "include/maniac/accuracy.h"
#include <cmath>
#include <random>

namespace maniac {

double AccuracyMultiStddev::sample(std::mt19937& rng, double density, const config::AccuracyParams& params) {
    double scale = 1.0 + (density * params.density_scaling);
    double inner = params.inner_stddev * scale;
    double mid = params.mid_stddev * scale;
    double outer = params.outer_stddev * scale;

    std::uniform_real_distribution<> dis(0.0, 1.0);
    double r = dis(rng);
    if (r < params.outer_chance) {
        std::normal_distribution<> nd(0.0, outer);
        return nd(rng);
    } else if (r < params.mid_chance + params.outer_chance) {
        std::normal_distribution<> nd(0.0, mid);
        return nd(rng);
    } else {
        std::normal_distribution<> nd(0.0, inner);
        return nd(rng);
    }
}

double RandomWalkDrift::update(std::mt19937& rng, const config::RandomWalkParams& params) {
    if (!params.enabled) return current_drift;
    std::uniform_real_distribution<> step(-params.walk_strength, params.walk_strength);
    current_drift += step(rng);
    current_drift *= (1.0 - params.pull_strength);
    if (current_drift > 30.0) current_drift = 30.0;
    if (current_drift < -30.0) current_drift = -30.0;
    return current_drift;
}

} // namespace maniac
