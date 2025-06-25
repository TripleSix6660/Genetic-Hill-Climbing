#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include "../globals.h"
#include "../utilities/individual.h"

namespace hill_climbing {
    inline constexpr float spacing_x = 75.f;
    inline constexpr float top_y = 100.f;
    inline constexpr float text_offset_y = -25.f;
    inline const vector2 bitmap_size(globals::image_width * 5.f, globals::image_height * 5.f);

    inline const std::chrono::seconds pause_duration(10);

    inline constexpr unsigned int interval = 15'000U;
    inline constexpr unsigned int failure_threshold = 90'000U;
    inline constexpr unsigned int genome_size = globals::image_width * globals::image_height * 4;
    //inline float mutation_rate = 0.0001f;

    inline individual color_current;
    inline individual gray_scale_current;
    inline individual binary_current;

    inline std::atomic<bool> color_running{ true };
    inline std::atomic<bool> gray_scale_running{ true };
    inline std::atomic<bool> binary_running{ true };

    inline std::atomic<unsigned int> color_generation{ 0 };
    inline std::atomic<unsigned int> gray_scale_generation{ 0 };
    inline std::atomic<unsigned int> binary_generation{ 0 };

    inline std::mutex best_color_mtx;
    inline std::mutex best_gray_scale_mtx;
    inline std::mutex best_binary_mtx;

    inline individual color_best;
    inline individual gray_scale_best;
    inline individual binary_best;

	inline unsigned int color_failure_count = 0;
	inline unsigned int gray_scale_failure_count = 0;
	inline unsigned int binary_failure_count = 0;

    void initialize();

    void initialize_color();
    void initialize_gray_scale();
    void initialize_binary();

    void color_worker();
    void gray_scale_worker();
    void binary_worker();

    void mutate_color(individual& ind, const float& mutation_rate);
    void mutate_grayscale(individual& ind, const float& mutation_rate);
    void mutate_binary(individual& ind, const float& mutation_rate);

    float calculate_fitness_color(const individual& ind);
    float calculate_fitness_gray_scale(const individual& ind);
    float calculate_fitness_binary(const individual& ind);
}
