#include "timer.hpp"
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

int f(int loop) {
    for (int i = 0; i < loop; i ++) {
        time_nested("10ms", [&]() {
            this_thread::sleep_for(chrono::milliseconds(10));
        });
        time_nested("y", [&]() {
            this_thread::sleep_for(chrono::milliseconds(1));
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
    vector<thread> spawned_threads;
    static thread_local int thread_id = 0;
    static atomic<int> finished = 0;
    static atomic<bool> stop = false;
    for (int i = 1; i < 4; i ++) {
        spawned_threads.emplace_back([&, i]() {
            init_timer();
            thread_id = i;  // thread-local write
            test();
            finished++;
            if (thread_id == 1) {
                while (finished < 3) {
                    this_thread::sleep_for(chrono::microseconds(100));
                }
                print_all_timers(timer::print_type::pt_full);
            }
        });
    }
    for (int i = 1; i < 4; i++) {
        spawned_threads[i - 1].join();
    }
    return 0;
}