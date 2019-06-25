#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <vector>
#include <png.h>
#include "Color.hpp"
#include "Point.hpp"
#include "Utils.hpp"

using namespace std::chrono_literals;

const long double min_x = -2.L;
const long double max_x = 1.L;
const long double min_y = -1.125L;
const long double max_y = 1.125L;
const long double x_skip = 0.001L;
const long double y_skip = 0.001L;
const size_t num_xs = (max_x - min_x)/x_skip;
const size_t num_ys = (max_y - min_y)/y_skip;

std::vector<Color> data(num_xs * num_ys * 3, Color(0, 0, 0));
std::mutex data_mutex;

const size_t min_iterations = 10000;
const size_t max_iterations = 100000; // 5000000? 1/10 prob?

const double sample_probability = 1.L/4.L; // The probability a random point will be sampled
std::vector<Point> samples;
std::mutex samples_mutex;

const size_t num_sections = std::thread::hardware_concurrency(); // The number of cores
const size_t section_offset = num_ys/num_sections;

std::vector<long double> progresses;
auto update_freq = 100ms; // Update the user every 100ms
int num_update_cols = 40; // Number of columns in the progress bar
std::atomic<size_t> sections_completed = 0;

const size_t num_colors = 7;
const ColorHSV colors[num_colors] = {ColorHSV(  0, 1.f, 1.f), ColorHSV( 30, 1.f, 1.f), ColorHSV( 90, 1.f, 1.f),
                                     ColorHSV(180, 1.f, 1.f), ColorHSV(200, 1.f, 1.f), ColorHSV(260, 1.f, 1.f),
                                     ColorHSV(300, 1.f, 1.f)};

void sample_orbits(size_t section) {
    long double area = static_cast<long double>(section_offset * num_xs); // The area this thread will cover

    std::default_random_engine generator;
    generator.seed(0); // Seed 0 for now
    std::uniform_real_distribution<double> distribution(0, 1.L/sample_probability);

    for (size_t y_idx = section; y_idx < num_ys; y_idx += num_sections) {
        for (size_t x_idx = 0; x_idx < num_xs; ++x_idx) {
            progresses[section] += 1.L/area;
            if (distribution(generator) > 1.L) {
                continue;
            }

            const long double a = x_idx*x_skip + min_x;
            const long double b = y_idx*y_skip + min_y;

            // Check if we are in the period-2 bulb or the cardioid
            if ((b+1)*(b+1) + a*a <= 1.L/16.L) continue;
            const long double q = std::powl((b - 1.L/4.L), 2) + b*b;
            if ((q * (q + a - 1.L/4.L)) <= 1.L/4.L * b*b) continue;

            // Start iterating
            long double new_a = a;
            long double new_b = b;
            for (size_t i = 0; i < max_iterations; ++i) {
                long double temp_a = new_a;
                long double temp_b = new_b;
                new_a = temp_a*temp_a - temp_b*temp_b + a;
                new_b = 2*temp_a*temp_b + b;

                if (new_a*new_a + new_b*new_b >= 4) {
                    if (i >= min_iterations) {
                        std::lock_guard<std::mutex> lock(samples_mutex);
                        samples.push_back(Point(x_idx, y_idx));
                    }
                    break;
                }
            }
        }
    }
    ++sections_completed;
}

void compute_section(size_t section) {
    long double area = static_cast<long double>(section_offset * num_xs); // The area this thread will cover
    size_t color_idx = 0;

    for (size_t y_idx = section; y_idx < num_ys; y_idx += num_sections) {
        progresses[section] += static_cast<long double>(num_xs)/area;
        auto point = find_first_y(samples.begin(), samples.end(), y_idx);
        if (point == samples.end()) continue; // We did not find a y value

        for (; point->y == y_idx; ++point) {            
            data[y_idx*num_xs + point->x] += Color(1, 1, 1);
            const long double a = point->x*x_skip + min_x;
            const long double b = y_idx*y_skip + min_y;

            long double new_a = a;
            long double new_b = b;
            for (size_t i = 0; i < max_iterations; ++i) {
                const long double temp_a = new_a;
                const long double temp_b = new_b;
                new_a = temp_a*temp_a - temp_b*temp_b + a;
                new_b = 2*temp_a*temp_b + b;

                if (new_a*new_a + new_b*new_b >= 4) break;
                if (new_a < min_x || new_a > max_x || new_b < min_y || new_b > max_y) break;

                const size_t x = (new_a - min_x)/x_skip;
                const size_t y = (new_b - min_y)/y_skip;
                const size_t idx = y*num_xs + x;
                if (idx < 0 || idx >= data.size()) continue;
                data[y*num_xs + x] = static_cast<Color>(colors[color_idx]);
            }
            color_idx = (color_idx+1)%num_colors;
        }
    }
    ++sections_completed;
}

void notify_user() {
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < num_sections+1; ++i) {
        std::cout << "\n";
    }

    while (true) {
        // Go to the top
        std::cout << "\033[" << num_sections + 1 << "A";

        for (size_t section = 0; section < num_sections; ++section) {
            std::cout << "\r\033[K"; // Clear the line
            std::cout << "Section " << section + 1 << ": [";

            int prog_bar_fill = static_cast<int>(std::round(progresses[section] * num_update_cols));
            std::cout << std::setfill('#') << std::setw(prog_bar_fill) << "";
            std::cout << std::setfill(' ') << std::setw(num_update_cols - prog_bar_fill) << "";

            std::cout << "] (" << std::setprecision(progresses[section] < 0.1 ? 3 : 4) << progresses[section] * 100 << "% done)\n";
        }
        std::cout << "\r\033[K\033[1mTotal:     [";

        long double total = std::accumulate(progresses.begin(), progresses.end(), 0.f);
        total /= static_cast<long double>(num_sections);

        int prog_bar_fill = static_cast<int>(std::round(total * num_update_cols));
        std::cout << std::setfill('#') << std::setw(prog_bar_fill) << "";
        std::cout << std::setfill(' ') << std::setw(num_update_cols - prog_bar_fill) << "";
        std::cout << "] (" << std::setprecision(total < 0.1 ? 3 : 4) << total * 100 << "% done)\033[0m\n";

        std::chrono::duration<long double> duration = std::chrono::high_resolution_clock::now() - start_time;
        int hours = duration.count()/3600;
        int mins = duration.count()/60 - hours*60;
        int secs = duration.count() - mins*60 - hours*3600;

        long double least_area = *std::min_element(progresses.begin(), progresses.end());
        int time_rem = duration.count()/least_area - duration.count();
        int hours_rem = time_rem/3600;
        int mins_rem = time_rem/60 - hours_rem*60;
        int secs_rem = time_rem - mins_rem*60 - hours_rem*3600;

        std::cout << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << mins << ":" << std::setw(2) << secs << " ("; 
        std::cout << std::setw(2) << hours_rem << ":" << std::setw(2) << mins_rem << ":" << std::setw(2) << secs_rem << " remaining)" << std::flush;

        if (sections_completed == num_sections) {
            break;
        }
        std::this_thread::sleep_for(100ms);
    }
    std::cout << "\n" << std::endl;
}

int main() {
    std::cout << "\033[?25l"; // Hide the cursor

    progresses = std::vector<long double>(num_sections, 0.L);

    // Find the samples
    std::cout << "\033[1mSampling\033[0m" << std::endl;
    std::vector<std::thread> sample_threads;
    sample_threads.reserve(num_sections);
    for (size_t i = 0; i < num_sections; ++i) {
        sample_threads.push_back(std::thread(sample_orbits, i));
    }
    std::thread notify_sample_thread(notify_user);

    // Wait for the sampling to finish
    for (auto it = sample_threads.begin(); it != sample_threads.end(); ++it) {
        it->join();
    }
    notify_sample_thread.join();

    sections_completed = 0;
    progresses = std::vector<long double>(num_sections, 0.L);

    // Sort the samples by y coordinate (x coordinate does not matter)
    std::sort(samples.begin(), samples.end(), [](Point a, Point b) -> bool {
        return a.y < b.y;
    });

    // Do the computations
    std::cout << "\033[1mComputing\033[0m" << std::endl;
    std::vector<std::thread> compute_threads;
    compute_threads.reserve(num_sections);
    for (size_t i = 0; i < num_sections; ++i) {
        compute_threads.push_back(std::thread(compute_section, i));
    }
    std::thread notify_compute_thread(notify_user);

    // Wait for all the computations to be finished;
    for (auto it = compute_threads.begin(); it != compute_threads.end(); ++it) {
        it->join();
    }
    notify_compute_thread.join();

    std::cout << "\r\033[?25h" << std::endl; // Show the cursor again

    // Save the image
    write_png("test.png", num_xs, num_ys, data);
    system("open test.png");

    return 0;
}