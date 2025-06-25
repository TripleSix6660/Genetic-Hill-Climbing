#include <thread>
#include "hill_climbing.h"
#include "../utilities/random.h"
#include <algorithm>

void hill_climbing::mutate_color(individual& ind, const float& mutation_rate) {
    for (auto& genome : ind.genome) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            genome = random_int(0, 255);
        }
    }
}

void hill_climbing::mutate_grayscale(individual& ind, const float& mutation_rate) {
    for (size_t i = 0; i < ind.genome.size(); i += 4) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            uint8_t gray = random_int(0, 255);
            ind.genome[i] = gray;
            ind.genome[i + 1] = gray;
            ind.genome[i + 2] = gray;
        }
    }
}

void hill_climbing::mutate_binary(individual& ind, const float& mutation_rate) {
    for (size_t i = 0; i < ind.genome.size(); i += 4) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            uint8_t val = (ind.genome[i] == 0) ? 255 : 0;
            ind.genome[i] = val;
            ind.genome[i + 1] = val;
            ind.genome[i + 2] = val;
        }
    }
}

float hill_climbing::calculate_fitness_color(const individual& ind) {
    const size_t bitmap_size = globals::image_byte_code_vector.size();
    double total_fitness = 0.0;

    for (size_t i = 0; i < bitmap_size; ++i) {
        int delta = static_cast<int>(ind.genome[i]) - static_cast<int>(globals::image_byte_code_vector[i]);
        total_fitness += std::abs(delta);
    }

    total_fitness /= bitmap_size;
    return static_cast<float>(1.0 - (total_fitness / 255.0));
}

float hill_climbing::calculate_fitness_gray_scale(const individual& ind) {
    const size_t bitmap_size = globals::image_byte_code_vector.size() / 4;
    double total_fitness = 0.0;

    for (size_t i = 0; i < bitmap_size; ++i) {
        const uint8_t r1 = globals::image_byte_code_vector[i * 4 + 0];
        const uint8_t g1 = globals::image_byte_code_vector[i * 4 + 1];
        const uint8_t b1 = globals::image_byte_code_vector[i * 4 + 2];

        const uint8_t r2 = ind.genome[i * 4 + 0];
        const uint8_t g2 = ind.genome[i * 4 + 1];
        const uint8_t b2 = ind.genome[i * 4 + 2];

        const float gray1 = 0.114f * r1 + 0.587f * g1 + 0.299f * b1;
        const float gray2 = 0.114f * r2 + 0.587f * g2 + 0.299f * b2;

        const float delta = gray2 - gray1;
        total_fitness += std::abs(delta);
    }

    return static_cast<float>(1.0 - (total_fitness / (bitmap_size * 255.0)));
}

float hill_climbing::calculate_fitness_binary(const individual& ind) {
    const size_t bitmap_size = globals::image_byte_code_vector.size() / 4;
    unsigned int total_fitness = 0U;

    for (size_t i = 0; i < bitmap_size; ++i) {
        const uint8_t r1 = globals::image_byte_code_vector[i * 4 + 0];
        const uint8_t g1 = globals::image_byte_code_vector[i * 4 + 1];
        const uint8_t b1 = globals::image_byte_code_vector[i * 4 + 2];

        const uint8_t r2 = ind.genome[i * 4 + 0];
        const uint8_t g2 = ind.genome[i * 4 + 1];
        const uint8_t b2 = ind.genome[i * 4 + 2];

        const float gray1 = 0.114f * r1 + 0.587f * g1 + 0.299f * b1;
        const float gray2 = 0.114f * r2 + 0.587f * g2 + 0.299f * b2;

        const bool binary1 = gray1 > 128.0f;
        const bool binary2 = gray2 > 128.0f;

        if (binary1 == binary2)
            total_fitness++;
    }

    return static_cast<float>(static_cast<float>(total_fitness) / static_cast<float>(bitmap_size));
}

template <typename FitnessFunc, typename MutateFunc, typename InitFunc>
void worker_loop(individual& current, individual& best, std::mutex& mtx, unsigned int& failure_count,std::atomic<bool>& running, std::atomic<unsigned int>& iteration, FitnessFunc fitness_func, MutateFunc mutate_func, InitFunc init_func) {
    
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    reset : 
    init_func();

    while (running) {
        individual neighbor = current;
        mutate_func(neighbor, random_float(0.00001f, 0.001f));
        float neighbor_fitness = fitness_func(neighbor);

        if (neighbor_fitness > current.fitness) {
            current = neighbor;
            current.fitness = neighbor_fitness;

            {
                std::lock_guard<std::mutex> lock(mtx);
                if (current.fitness > best.fitness) {
                    best = current;
                }
            }
        }
        else 
            failure_count++;

        if (failure_count >= hill_climbing::failure_threshold)
        {
            failure_count = 0;
            goto reset;
        }

        iteration++;

        if (iteration % hill_climbing::interval == 0)
            std::this_thread::sleep_for(hill_climbing::pause_duration);
    }
}

void hill_climbing::color_worker() {
    worker_loop(color_current, color_best, best_color_mtx, color_failure_count, color_running, color_generation, calculate_fitness_color, mutate_color, initialize_color);
}

void hill_climbing::gray_scale_worker() {
    worker_loop(gray_scale_current, gray_scale_best, best_gray_scale_mtx, gray_scale_failure_count, gray_scale_running, gray_scale_generation, calculate_fitness_gray_scale, mutate_grayscale, initialize_gray_scale);
}

void hill_climbing::binary_worker() {
    worker_loop(binary_current, binary_best, best_binary_mtx, binary_failure_count, binary_running, binary_generation, calculate_fitness_binary, mutate_binary, initialize_binary);
}

void hill_climbing::initialize_color() {
    color_current.genome.resize(genome_size);
    for (auto& genome : color_current.genome) {
        genome = random_int(0, 255);
    }
    color_current.fitness = calculate_fitness_color(color_current);
    color_best = color_current;
}

void hill_climbing::initialize_gray_scale() {
    gray_scale_current.genome.resize(genome_size);
    for (size_t i = 0; i < genome_size; i += 4) {
        uint8_t gray = random_int(0, 255);
        gray_scale_current.genome[i] = gray;
        gray_scale_current.genome[i + 1] = gray;
        gray_scale_current.genome[i + 2] = gray;
        gray_scale_current.genome[i + 3] = 255;
    }
    gray_scale_current.fitness = calculate_fitness_gray_scale(gray_scale_current);
    gray_scale_best = gray_scale_current;
}

void hill_climbing::initialize_binary() {
    binary_current.genome.resize(genome_size);
    for (size_t i = 0; i < genome_size; i += 4) {
        uint8_t val = random_int(0, 1) * 255;
        binary_current.genome[i] = val;
        binary_current.genome[i + 1] = val;
        binary_current.genome[i + 2] = val;
        binary_current.genome[i + 3] = 255;
    }

    binary_current.fitness = calculate_fitness_binary(binary_current);
    binary_best = binary_current;
}

void hill_climbing::initialize() {
	initialize_color();
    initialize_gray_scale();
    initialize_binary();

    std::thread color_worker_thread(color_worker);
    std::thread gray_scale_worker_thread(gray_scale_worker);
    std::thread binary_worker_thread(binary_worker);

    MSG msg;
    while (true) {
        if (LI_FN(PeekMessageW)(&msg, renderer::overlay::hwnd, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) break;
            LI_FN(TranslateMessage)(&msg);
            LI_FN(DispatchMessageW)(&msg);
        }

        {
            std::lock_guard<std::mutex> lock(best_color_mtx);
            if (!color_best.genome.empty()) {
                globals::current_color_bitmap->CopyFromMemory(nullptr, color_best.genome.data(), globals::image_width * 4);
            }
        }

        {
            std::lock_guard<std::mutex> lock(best_gray_scale_mtx);
            if (!gray_scale_best.genome.empty()) {
                globals::current_gray_scale_bitmap->CopyFromMemory(nullptr, gray_scale_best.genome.data(), globals::image_width * 4);
            }
        }

        {
            std::lock_guard<std::mutex> lock(best_binary_mtx);
            if (!binary_best.genome.empty()) {
                globals::current_binary_bitmap->CopyFromMemory(nullptr, binary_best.genome.data(), globals::image_width * 4);
            }
        }

        renderer::d2d1::render_target->BeginDraw();
        renderer::d2d1::render_target->Clear();

        auto font = GET_FONT(encrypt("verdana"), 13.f, DWRITE_FONT_WEIGHT_BOLD);

        vector2 goal_pos(spacing_x, top_y);
        renderer::items::render_text(font, goal_pos + vector2(0.f, text_offset_y), L"Goal", COLOR_WHITE);
        renderer::items::render_bitmap(globals::goal_bitmap, goal_pos, goal_pos + bitmap_size, 1.f);

        if (color_generation.load()) {
            vector2 color_pos(goal_pos.x + globals::image_width + spacing_x * 5.f, top_y);
            std::wstring text = L"Color Gen - Fitness: " + std::to_wstring(color_best.fitness) +
                L" | Gen: " + std::to_wstring(color_generation.load());
            renderer::items::render_text(font, color_pos + vector2(0.f, text_offset_y), text.c_str(), COLOR_WHITE);
            renderer::items::render_bitmap(globals::current_color_bitmap, color_pos, vector2(color_pos.x + bitmap_size.x, color_pos.y + bitmap_size.y), 1.f);
        }

        if (gray_scale_generation.load()) {
            vector2 gray_pos(goal_pos.x + 2 * (globals::image_width + spacing_x * 5.f), top_y);
            std::wstring text = L"Gray Gen - Fitness: " + std::to_wstring(gray_scale_best.fitness) +
                L" | Gen: " + std::to_wstring(gray_scale_generation.load());
            renderer::items::render_text(font, gray_pos + vector2(0.f, text_offset_y), text.c_str(), COLOR_WHITE);
            renderer::items::render_bitmap(globals::current_gray_scale_bitmap, gray_pos, vector2(gray_pos.x + bitmap_size.x, gray_pos.y + bitmap_size.y), 1.f);
        }

        if (binary_generation.load()) {
            vector2 bin_pos(goal_pos.x + 3 * (globals::image_width + spacing_x * 5.f), top_y);
            std::wstring text = L"Binary Gen - Fitness: " + std::to_wstring(binary_best.fitness) +
                L" | Gen: " + std::to_wstring(binary_generation.load());
            renderer::items::render_text(font, bin_pos + vector2(0.f, text_offset_y), text.c_str(), COLOR_WHITE);
            renderer::items::render_bitmap(globals::current_binary_bitmap, bin_pos, vector2(bin_pos.x + bitmap_size.x, bin_pos.y + bitmap_size.y), 1.f);
        }

        renderer::d2d1::render_target->EndDraw();

        if (color_best.fitness >= 0.95)
            color_running.store(false);

        if (gray_scale_best.fitness >= 0.98)
            gray_scale_running.store(false);

        if (binary_best.fitness >= 1)
            binary_running.store(false);

        if (GetAsyncKeyState(VK_END) & 0x8000 || (!color_running && !gray_scale_running && !binary_running)) {
            LI_FN(PostQuitMessage)(0);
            ExitProcess(0);
            break;
        }
    }

    color_running.store(false);
    gray_scale_running.store(false);
    binary_running.store(false);
    color_worker_thread.join();
    gray_scale_worker_thread.join();
    binary_worker_thread.join();
}