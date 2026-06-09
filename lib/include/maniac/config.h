// --- lib/include/maniac/config.h ---
#pragma once
#include <string>
#include <unordered_map>

namespace maniac {

// Forward declarations for our new structs
struct AccuracyParams;
struct RandomWalkParams;
struct PanicParams;
struct LoggingParams;
struct ColumnBiasParams;

struct config {
    static constexpr int VERSION = 3;
    static constexpr auto STATIC_HUMANIZATION = 0;
    static constexpr auto DYNAMIC_HUMANIZATION = 1;

    // ===== Existing Settings =====
    int tap_time = 20;
    bool mirror_mod = false;
    int compensation_offset = -15;
    int humanization_modifier = 0;
    int randomization_mean = 0;
    int randomization_stddev = 0;
    int humanization_type = DYNAMIC_HUMANIZATION;
    std::string keys = "asdfjkl;";

    // ===== NEW: Accuracy Multi-Stddev Settings =====
    struct AccuracyParams {
        double inner_stddev = 5.0;
        double mid_stddev = 10.0;
        double mid_chance = 0.30;
        double outer_stddev = 20.0;
        double outer_chance = 0.10;
        double density_scaling = 0.50;
        bool enabled = true; // Toggle for this feature
    } accuracy;

    // ===== NEW: Random Walk Drift Settings =====
    struct RandomWalkParams {
        bool enabled = true;
        double walk_strength = 3.0;
        double pull_strength = 0.030;
    } random_walk;

    // ===== NEW: Panic Mode Settings =====
    struct PanicParams {
        bool enabled = false;
        double panic_window_ms = 500.0;
        int panic_threshold_notes = 8;
        double panic_chance = 0.15;
        double miss_chance = 0.30;
        double extra_press_chance = 0.10;
    } panic;

    // ===== NEW: Logging Settings =====
    struct LoggingParams {
        bool enabled = false;
        std::string file_path = "maniac_log.csv";
        bool log_json = false;
    } logging;

    // ===== NEW: Per-Column Bias =====
    std::unordered_map<int, double> column_biases;
};

// Function to load/save the new, expanded config
bool read_from_file(config& cfg, const std::string& path = "maniac_config.json");
bool write_to_file(const config& cfg, const std::string& path = "maniac_config.json");

} // namespace maniac
