#include "timer.hpp"
#include <iostream>
#include <thread>
using namespace std;

int f(int loop_count) {
    int x = 0;
    for (int i = 0; i < 1e1; i ++) {
        time_nested("x", [&]() {
            for (int j = 0; j < loop_count; j ++) {
                x += (i + 1) * (j + 1);
            }
        });
        time_nested("y", [&]() {
            x = x ^ (x + x);
            this_thread::sleep_for(chrono::milliseconds(10));
        });
    }
    return x;
}

int main() {
    timer::active = true;
    time_nested("a", [&]() {
        for (int i = 0; i < 1e1; i++) {
            f(1e6);
        }
    });
    time_nested("b", [&]() {
        for (int i = 0; i < 1e1; i++) {
            f(1e5);
        }
    });
    print_all_timers(timer::print_type::pt_full);
    return 0;
}