#include "timer.hpp"
#include <iostream>
#include <thread>
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

int main() {
    timer::active = true;
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
    print_all_timers(timer::print_type::pt_full);
    return 0;
}