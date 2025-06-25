#include "genetic.h"
#include <thread>
#include <algorithm>
#include "../utilities/random.h"


void genetic_algorithm::mutate_color(individual& ind, const float& mutation_rate) {
    for (auto& genome : ind.genome) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            genome = random_int(0, 255);
        }
    }
}

void genetic_algorithm::mutate_grayscale(individual& ind, const float& mutation_rate) {
    for (size_t i = 0; i < ind.genome.size(); i += 4) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            uint8_t gray = random_int(0, 255);
            ind.genome[i] = gray;
            ind.genome[i + 1] = gray;
            ind.genome[i + 2] = gray;
        }
    }
}

void genetic_algorithm::mutate_binary(individual& ind, const float& mutation_rate) {
    for (size_t i = 0; i < ind.genome.size(); i += 4) {
        if (random_float(0.f, 1.f) < mutation_rate) {
            uint8_t val = (ind.genome[i] == 0) ? 255 : 0;
            ind.genome[i] = val;
            ind.genome[i + 1] = val;
            ind.genome[i + 2] = val;
        }
    }
}

individual genetic_algorithm::generate_offspring(const std::vector<individual>& population) {
    individual parent1 = population[0];
    individual parent2 = population[0];

    for (size_t i = 1; i < static_cast<size_t>(population.size() - random_int(0, random_int(8, 16))); ++i) {
        if (population[i].fitness > parent1.fitness) {
            parent1 = population[i];
        }
    }

    for (size_t i = 1; i < static_cast<size_t>(population.size() - random_int(0, random_int(8, 16))); ++i) {
        if (population[i].fitness > parent2.fitness && population[i].fitness != parent1.fitness) {
            parent2 = population[i];
        }
    }

    parent1.fitness = 0.f;
    parent2.fitness = 0.f;

    return crossover(parent1, parent2);
}

individual genetic_algorithm::crossover(const individual& parent1, const individual& parent2) {
    individual offspring;
    offspring.genome.resize(parent1.genome.size());
    size_t crossover_point = random_int(0, parent1.genome.size() - 1);
    for (size_t i = 0; i < parent1.genome.size(); ++i) {
        offspring.genome[i] = (i < crossover_point) ? parent1.genome[i] : parent2.genome[i];
    }
    return offspring;
}

float genetic_algorithm::calculate_fitness_color(const individual& ind) {
    const size_t bitmap_size = globals::image_byte_code_vector.size();
    double total_fitness = 0.0;

    for (size_t i = 0; i < bitmap_size; ++i) {
        int delta = static_cast<int>(ind.genome[i]) - static_cast<int>(globals::image_byte_code_vector[i]);
        total_fitness += std::abs(delta);
    }

    total_fitness /= bitmap_size;
    return static_cast<float>(1.0 - (total_fitness / 255.0));
}

float genetic_algorithm::calculate_fitness_gray_scale(const individual& ind) {
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

float genetic_algorithm::calculate_fitness_binary(const individual& ind) {
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

template <typename FitnessFunc, typename MutateFunc>
void worker_loop(std::vector<individual>& population, individual& best, std::mutex& mtx, std::atomic<bool>& running, std::atomic<unsigned int>& generation, FitnessFunc fitness_func, MutateFunc mutate_func) {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    while (running) {
        for (auto& ind : population) {
            ind.fitness = fitness_func(ind);
        }

        individual current_best = population.front();
        for (const auto& ind : population) {
            if (ind.fitness > current_best.fitness) {
                current_best = ind;
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtx);
            if (current_best.fitness > best.fitness) {
                best = current_best;
            }
        }
        std::vector<individual> new_population;
        new_population.reserve(genetic_algorithm::population_size);
        new_population.push_back(best);
        while (new_population.size() < genetic_algorithm::population_size) {
            individual offspring = genetic_algorithm::generate_offspring(population);
            mutate_func(offspring, random_float(0.00001f, 0.001f));
            new_population.push_back(offspring);
        }
        population = std::move(new_population);
        generation++;

        if (generation % genetic_algorithm::interval == 0)
			std::this_thread::sleep_for(genetic_algorithm::pause_duration);
    }
}

void genetic_algorithm::color_worker() {
    worker_loop(color_population, color_best, best_color_mtx, color_running, color_generation, calculate_fitness_color, mutate_color);
}

void genetic_algorithm::gray_scale_worker() {
    worker_loop(gray_scale_population, gray_scale_best, best_gray_scale_mtx, gray_scale_running, gray_scale_generation, calculate_fitness_gray_scale, mutate_grayscale);
}

void genetic_algorithm::binary_worker() {
    worker_loop(binary_population, binary_best, best_binary_mtx, binary_running, binary_generation, calculate_fitness_binary, mutate_binary);
}

void genetic_algorithm::initialize() {
    for (auto& ind : color_population) {
        ind.genome.resize(genome_size);
        for (auto& genome : ind.genome) {
            genome = random_int(0, 255);
        }
    }
    for (auto& ind : gray_scale_population) {
        ind.genome.resize(genome_size);
        for (size_t i = 0; i < genome_size; i += 4) {
            uint8_t gray = random_int(0, 255);
            ind.genome[i] = gray;
            ind.genome[i + 1] = gray;
            ind.genome[i + 2] = gray;
            ind.genome[i + 3] = 255;
        }
    }
    for (auto& ind : binary_population) {
        ind.genome.resize(genome_size);
        for (size_t i = 0; i < genome_size; i += 4) {
            uint8_t val = random_int(0, 1) * 255;
            ind.genome[i] = val;
            ind.genome[i + 1] = val;
            ind.genome[i + 2] = val;
            ind.genome[i + 3] = 255;
        }
    }

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

        if (color_best.fitness >= 0.82)
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