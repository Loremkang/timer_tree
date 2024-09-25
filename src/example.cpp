#include "timer.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <atomic>

int f(int loop) {
    for (int i = 0; i < loop; i ++) {
        time_nested("10ms", [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10));
        });
        time_nested("y", [&]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        });
    }
    return 0;
}

inline void test() {
    time_nested("a", [&]() {
        for (int i = 0; i < 1e1; i++) {
            f(10);
        }
    });
    time_nested("b", [&]() {
        for (int i = 0; i < 1e1; i++) {
             f(5);
        }
    });
}

int main() {
    int tid = 0;
    std::vector<std::thread> spawned_threads;
    thread_local int thread_id = 0;

    init_root_timer();
    timer::active = true;
    timer::default_detail = true;
    timer::print_when_time = true;

    time_nested_pass("main", [&](timer* timer) {
        for (int i = 1; i < 4; i ++) {
            spawned_threads.emplace_back([&, i]() {
                thread_id = i;  // thread-local write
                time_nested<true>("thread " + std::to_string(i), [&]() {
                    test();
                }, timer);
            });
        }
        for (int i = 1; i < 4; i++) {
            spawned_threads[i - 1].join();
        }
    });

    timer::active = false;
    print_all_timers(print_type::pt_full);

    return 0;
}