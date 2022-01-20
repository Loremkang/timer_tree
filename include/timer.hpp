#pragma once
#include <ctime>
#include <cstdio>
#include <string>
#include <iostream>
#include <chrono>
#include <set>
#include <map>
#include <vector>
#include <cassert>
#include <iomanip>
using namespace std;
using namespace std::chrono;

class timer {
   public:
    static bool active, default_detail;
    static timer* current_timer;
    static timer* root_timer;

    enum print_type { pt_full, pt_time, pt_name };

    string name;
    timer* parent;

    string name_with_prefix;
    duration<double> total_time;
    int count;
    high_resolution_clock::time_point start_time, end_time;
    vector<double> details;
    map<string, timer*> sub_timers;

    timer(string _name, timer* _parent) {
        name = _name;
        parent = _parent;
        if (parent != nullptr) {
            name_with_prefix = parent->name_with_prefix + " -> " + name;
        } else {
            name_with_prefix = name;
        }
        total_time = duration<double>();
        count = 0;
        details.clear();
        sub_timers.clear();
    }

    ~timer() {
        details.clear();
        for (auto& sub_timer_pairs : sub_timers) {
            delete sub_timer_pairs.second;
        }
        sub_timers.clear();
    }

    void print(print_type pt) {
        if (count == 0) return;
        if (pt == pt_name) {
            cout << name_with_prefix << endl;
        } else if (pt == pt_time) {
            printf("Average Time: %lf\n", total_time.count() / this->count);
        } else {
            cout << "/----------------------------------------\\" << endl;
            cout << "Timer" << name_with_prefix << ": " << endl;
            cout << setw(20) << "Average Time"
                 << " : " << total_time.count() / this->count << endl;
            cout << setw(20) << "Total Time"
                 << " : " << total_time.count() << endl;
            printf("Proportion: \n");
            double total_proportion = 1.0;
            for (auto& sub_timer_pair : sub_timers) {
                auto sub_timer = sub_timer_pair.second;
                double sub_time = sub_timer->total_time.count();
                double sub_average_time = sub_time / sub_timer->count;
                double proportion = sub_time / total_time.count();
                total_proportion -= proportion;
                int proportion_int = (int)(proportion * 100.0);

                string sub_name = sub_timer_pair.second->name;
                cout << std::left << " -> " << std::setw(16) << sub_name
                     << " : " << std::setw(3) << proportion_int << "%" << " : " << sub_average_time << endl;
            }
            total_proportion = max(0.0, total_proportion);
            cout << std::left << " -> " << std::setw(16) << "other"
                 << " : " << total_proportion << endl;
            printf("Details: ");
            for (size_t i = 0; i < details.size(); i++) {
                printf("%lf ", details[i]);
            }
            cout << endl;
            printf("Occurance: %d\n", count);
            cout << "\\----------------------------------------/" << endl << endl;
        }
        fflush(stdout);
    }

    void start() { start_time = high_resolution_clock::now(); }
    void end(bool detail) {
        if (active) {
            end_time = high_resolution_clock::now();
            auto d = duration_cast<duration<double>>(end_time - start_time);
            total_time += d;
            count++;
            if (detail) {
                details.push_back(d.count());
            }
        }
    }
    void reset() {
        total_time = duration<double>();
        count = 0;
        details.clear();
    }
};

extern inline timer* get_root_timer() {
    static timer root_timer("", nullptr);
    return &root_timer;
}

bool timer::active = true;
bool timer::default_detail = true;
timer* timer::current_timer = get_root_timer();
timer* timer::root_timer = get_root_timer();

inline void time_start(string name) {
    timer* previous_timer = timer::current_timer;
    if (!previous_timer->sub_timers.count(name)) {
        timer* tt = new timer(name, previous_timer);
        previous_timer->sub_timers[name] = tt;
    }
    timer* t = previous_timer->sub_timers[name];
    t->start();
    timer::current_timer = t;
}

inline void time_end(string name, bool detail = timer::default_detail) {
    timer*t = timer::current_timer;
    assert(t == t->parent->sub_timers[name]);
    t->end(detail);
    timer::current_timer = t->parent;
}

template <class F>
inline void time_nested(string name, F f, bool detail = timer::default_detail) {
    // timer* previous_timer = timer::current_timer;
    // if (!previous_timer->sub_timers.count(name)) {
    //     timer* tt = new timer(name, previous_timer);
    //     previous_timer->sub_timers[name] = tt;
    // }
    // timer* t = previous_timer->sub_timers[name];
    // t->start();
    // timer::current_timer = t;
    time_start(name);
    f();
    time_end(name, detail);
    // t->end(detail);
    // timer::current_timer = previous_timer;
}

template <class F>
inline void apply_to_timers_recursive(timer* t, F f) {
    assert(t != nullptr);
    f(t);
    for (auto& timer : t->sub_timers) {
        apply_to_timers_recursive(timer.second, f);
    }
}

inline void print_all_timers(timer::print_type pt) {
    timer* root_timer = get_root_timer();
    apply_to_timers_recursive(root_timer, [&](timer* t) { t->print(pt); });
}

inline void reset_all_timers() {
    timer* root_timer = get_root_timer();
    apply_to_timers_recursive(root_timer, [&](timer* t) { t->reset(); });
}
