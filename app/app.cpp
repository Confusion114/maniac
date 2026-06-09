#include "window.h"
#include "config.h"
#include <maniac/maniac.h>

static void help_marker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 30.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

static void horizontal_break() {
    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::Separator();

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
}

static void set_priority_class(int priority) {
    const auto proc = GetCurrentProcess();
    const auto old_priority = GetPriorityClass(proc);

    SetPriorityClass(proc, priority);

    debug("changed priority class from 0x%lx to 0x%lx", old_priority,
            GetPriorityClass(proc));
}

int main(int, char **) {
    std::string message;

    config::read_from_file(maniac::config);

    auto run = [&message](osu::Osu &osu) {
        maniac::osu = &osu;

        message = "waiting for beatmap...";

        maniac::block_until_playing();

        message = "found beatmap";

        std::vector<osu::HitObject> hit_objects;

        for (int i = 0; i < 10; i++) {
            try {
                hit_objects = osu.get_hit_objects();

                break;
            } catch (std::exception &err) {
                debug("get hit objects attempt %d failed: %s", i + 1, err.what());

                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
        }

        if (hit_objects.empty()) {
            throw std::runtime_error("failed getting hit objects");
        }

        set_priority_class(HIGH_PRIORITY_CLASS);

        maniac::randomize(hit_objects, maniac::config.randomization_mean, maniac::config.randomization_stddev);

        if (maniac::config.humanization_type == maniac::config::STATIC_HUMANIZATION) {
            maniac::humanize_static(hit_objects, maniac::config.humanization_modifier);
        }

        if (maniac::config.humanization_type == maniac::config::DYNAMIC_HUMANIZATION) {
            maniac::humanize_dynamic(hit_objects, maniac::config.humanization_modifier);
        }

        auto actions = maniac::to_actions(hit_objects, osu.get_game_time());

        message = "playing";

        maniac::play(actions);

        set_priority_class(NORMAL_PRIORITY_CLASS);
    };

    auto thread = std::jthread([&message, &run](const std::stop_token& token) {
        while (!token.stop_requested()) {
            try {
                auto osu = osu::Osu();

                while (!token.stop_requested()) {
                    run(osu);
                }
            } catch (std::exception &err) {
                (message = err.what()).append(" (retrying in 2 seconds)");

                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    });

    window::start([&message] {
        ImGui::Begin("maniac", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);

        ImGui::Text("Status: %s", message.c_str());
        horizontal_break();

        ImGui::Combo("Humanization Type", &maniac::config.humanization_type, "Static\0Dynamic (new)\0\0");
        ImGui::SameLine();
        help_marker("Static: Density calculated per 1s chunk and applied to all hit objects in that chunk. Dynamic: Density 1s 'in front' of each hit object, applied individually.");

        ImGui::InputInt("Humanization", &maniac::config.humanization_modifier, 0, 1000);
        ImGui::SameLine();
        help_marker("Advanced hit-time randomization based on hit density.");

        horizontal_break();

        ImGui::Text("Adds a random hit-time offset generated using a normal \ndistribution with given mean and standard deviation.");
        ImGui::Dummy(ImVec2(0.0f, 2.0f));

        ImGui::InputInt("Randomization Mean", &maniac::config.randomization_mean);
        ImGui::InputInt("Randomization Stddev", &maniac::config.randomization_stddev);

        horizontal_break();

        ImGui::InputInt("Compensation", &maniac::config.compensation_offset);
        ImGui::SameLine();
        help_marker("Adds constant value to all hit-times to compensate for input latency, slower processors, etc.");

        ImGui::Checkbox("Mirror Mod", &maniac::config.mirror_mod);

        ImGui::InputInt("Tap time", &maniac::config.tap_time);
        ImGui::SameLine();
        help_marker("How long a key is held down for a single keypress, in milliseconds.");

        horizontal_break();
        ImGui::Dummy(ImVec2(0.0f, 5.0f));

        ImGui::TextDisabled("maniac by fs-c, https://github.com/fs-c/maniac");

        if (ImGui::BeginTabBar("ManiacControlTabs")) {

    // Accuracy Tab
    if (ImGui::BeginTabItem("Accuracy")) {
        ImGui::Checkbox("Enable Multi-Stddev", &maniac::config.accuracy.enabled);
        ImGui::SliderDouble("Inner Stddev", &maniac::config.accuracy.inner_stddev, 0.5, 15.0, "%.2f ms");
        ImGui::SliderDouble("Mid Stddev", &maniac::config.accuracy.mid_stddev, 1.0, 30.0, "%.2f ms");
        ImGui::SliderDouble("Mid Chance", &maniac::config.accuracy.mid_chance, 0.0, 0.6, "%.2f");
        ImGui::SliderDouble("Outer Stddev", &maniac::config.accuracy.outer_stddev, 5.0, 60.0, "%.2f ms");
        ImGui::SliderDouble("Outer Chance", &maniac::config.accuracy.outer_chance, 0.0, 0.4, "%.2f");
        ImGui::SliderDouble("Density Scaling", &maniac::config.accuracy.density_scaling, 0.0, 1.0, "%.2f");
        ImGui::EndTabItem();
    }

    // Drift Tab
    if (ImGui::BeginTabItem("Drift")) {
        ImGui::Checkbox("Enable Random Walk", &maniac::config.random_walk.enabled);
        ImGui::SliderDouble("Walk Strength", &maniac::config.random_walk.walk_strength, 0.0, 10.0, "%.2f ms/step");
        ImGui::SliderDouble("Pull Strength", &maniac::config.random_walk.pull_strength, 0.0, 0.2, "%.4f");
        ImGui::EndTabItem();
    }

    // Panic Tab
    if (ImGui::BeginTabItem("Panic")) {
        ImGui::Checkbox("Enable Panic Mode", &maniac::config.panic.enabled);
        ImGui::SliderDouble("Panic Window", &maniac::config.panic.panic_window_ms, 100.0, 2000.0, "%.0f ms");
        ImGui::SliderInt("Panic Threshold", &maniac::config.panic.panic_threshold_notes, 3, 30);
        ImGui::SliderDouble("Panic Chance", &maniac::config.panic.panic_chance, 0.0, 1.0, "%.2f");
        ImGui::SliderDouble("Miss Chance", &maniac::config.panic.miss_chance, 0.0, 0.8, "%.2f");
        ImGui::SliderDouble("Extra Press Chance", &maniac::config.panic.extra_press_chance, 0.0, 0.5, "%.2f");
        ImGui::EndTabItem();
    }

    // Column Bias Tab
    if (ImGui::BeginTabItem("Column Bias")) {
        int num_cols = maniac::config.keys.length();
        for (int i = 0; i < num_cols; ++i) {
            ImGui::SliderDouble(("Col " + std::to_string(i+1)).c_str(),
                                &maniac::config.column_biases[i], -10.0, 10.0, "%.2f ms");
        }
        ImGui::EndTabItem();
    }

    // Logging Tab
    if (ImGui::BeginTabItem("Logging")) {
        ImGui::Checkbox("Enable Logging", &maniac::config.logging.enabled);
        ImGui::Checkbox("Log in JSON format", &maniac::config.logging.log_json);
        ImGui::InputText("File Path", &maniac::config.logging.file_path);
        ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
}

        ImGui::End();
    });

    config::write_to_file(maniac::config);

    thread.request_stop();

    return EXIT_SUCCESS;
}

