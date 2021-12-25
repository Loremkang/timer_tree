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

// class timer;

// static map<string, timer*> all_timers;
// vector<timer*> all_timers;
// set<string> all_timer_names;

class timer {
   public:
    static bool active;
    static timer* current_timer;
    static timer* root_timer;
    
    // static string current_prefix;
    // static string get_name_with_prefix(string _name, string _prefix = current_prefix) {
    //     return _prefix + string(" -> ") + _name;
    // }

    enum print_type { pt_full, pt_time, pt_name };

    string name;
    timer* parent;

    string name_with_prefix;
    duration<double> total_time;
    int count;
    // bool active;
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
        // active = default_active;
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
            // printf("Average Time: %lf\n", total_time.count() / this->count);
            cout << setw(20) << "Average Time"
                 << " : " << total_time.count() / this->count << endl;
            // printf("Total Time: %lf\n", total_time.count());
            cout << setw(20) << "Total Time"
                 << " : " << total_time.count() << endl;
            // printf("Total Time: %lf\n", total_time.count());
            printf("Proportion: \n");
            double total_proportion = 1.0;
            for (auto& sub_timer_pair : sub_timers) {
                double sub_time = sub_timer_pair.second->total_time.count();
                double proportion = sub_time / total_time.count();
                total_proportion -= proportion;
                string sub_name = sub_timer_pair.second->name;
                cout << std::left << " -> " << std::setw(10) << sub_name
                     << " : " << proportion << endl;
            }
            total_proportion = max(0.0, total_proportion);
            cout << std::left << " -> " << std::setw(10) << "other"
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

    // void activate(bool val) { active = val; }
    void start() { start_time = high_resolution_clock::now(); }
    void end(bool detail = false) {
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

bool timer::active = false;
timer* timer::current_timer = get_root_timer();
timer* timer::root_timer = get_root_timer();

// inline timer* start_timer(string name) {
//     string name_with_prefix = timer::get_name_with_prefix(name);
//     if (!all_timers.count(name_with_prefix)) {
//         timer* tt = new timer(name);
//     }
//     timer* t = all_timers[name];
//     t->start();
//     return t;
// }

// template <class F>
// inline void time_root(string basename, F f, bool detail = true) {
//     curname = basename;
//     cout << curname << endl;
//     if (!all_timers.count(curname)) {
//         timer* tt = new timer(curname);
//         tt->reset();
//         tt->turnon(timer_turnon);
//     }
//     timer* t = all_timers[curname];
//     // t->turnon(true);
//     t->start();
//     f();
//     t->end(detail);
// }

// #define INNER_TIMER (true)

template <class F>
inline void time_nested(string name, F f, bool detail = true) {
    timer* previous_timer = timer::current_timer;
    if (!previous_timer->sub_timers.count(name)) {
        timer* tt = new timer(name, previous_timer);
        previous_timer->sub_timers[name] = tt;
    }
    timer* t = previous_timer->sub_timers[name];
    t->start();
    timer::current_timer = t;
    f();
    t->end(detail);
    timer::current_timer = previous_timer;
}

// inline string time_nested_start(string deltaname, bool detail) {
//     string prename = curname;
//     curname = curname + string(" -> ") + deltaname;
//     cout << curname << endl;
//     if (!all_timers.count(curname)) {
//         timer* tt = new timer(curname);
//         tt->reset();
//         tt->turnon(timer_turnon);
//     }
//     timer* t = all_timers[curname];
//     t->start();
//     return prename;
// }

// inline void time_nested_end(string prename, bool detail = true) {
//     timer* t = all_timers[curname];
//     t->end(detail);
//     curname = prename;
// }

// template <class F>
// inline void time_f(string name, F f, bool detail = true) {
//     cout << name << endl;
//     if (!all_timers.count(name)) {
//         timer* tt = new timer(name);
//         tt->reset();
//         tt->turnon(timer_turnon);
//     }
//     timer* t = all_timers[name];
//     t->turnon(true);
//     t->start();
//     f();
//     t->end(detail);
// }

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

// inline void delete_all_timers() {
//     for (auto& timer : all_timers)  // access by reference to avoid copying
//     {
//         delete timer.second;
//     }
//     all_timers.clear();
// }
// timer send_task_timer("send_task");
// timer receive_task_timer("receive_task");
// timer execute_timer("execute");
// timer exec_timer("exec");