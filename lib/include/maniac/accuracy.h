#pragma once
#include <random>
#include "config.h"

namespace maniac {

struct AccuracyMultiStddev {
    static double sample(std::mt19937& rng, double density, const config::AccuracyParams& params);
};

struct RandomWalkDrift {
    double current_drift = 0.0;
    double update(std::mt19937& rng, const config::RandomWalkParams& params);
    void reset() { current_drift = 0.0; }
};

} // namespace maniac
