#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#define MAX_TIMERS 10
#define TIMER_NAME_LENGTH 32

typedef struct {
    absolute_time_t start_time;
    double elapsed_time;
    int active;
} Timer;

typedef struct {
    Timer timers[MAX_TIMERS];
    char timer_names[MAX_TIMERS][TIMER_NAME_LENGTH];
} TimerManager;

TimerManager* timer_manager_create();
void timer_set_name(TimerManager* manager, int index, const char* name);
void timer_start(TimerManager* manager, int index);
void timer_stop(TimerManager* manager, int index);
void timer_print(TimerManager* manager);
void timer_print_fps(TimerManager* manager, int index);
void timer_reset(TimerManager* manager);

TimerManager* timer_manager_create() {
    TimerManager* manager = malloc(sizeof(TimerManager));
    if (manager) {
        timer_reset(manager);
    }
    return manager;
}

void timer_set_name(TimerManager* manager, int index, const char* name) {
    if (index >= 0 && index < MAX_TIMERS) {
        strncpy(manager->timer_names[index], name, TIMER_NAME_LENGTH - 1);
        manager->timer_names[index][TIMER_NAME_LENGTH - 1] = '\0'; // Ensure null termination
    } else {
        printf("Invalid timer index: %d\n", index);
    }
}

void timer_start(TimerManager* manager, int index) {
    if (index >= 0 && index < MAX_TIMERS) {
        manager->timers[index].start_time = get_absolute_time();
        manager->timers[index].elapsed_time = 0;
        manager->timers[index].active = 1;
    } else {
        printf("Invalid timer index: %d\n", index);
    }
}

void timer_stop(TimerManager* manager, int index) {
    if (index >= 0 && index < MAX_TIMERS) {
        if (manager->timers[index].active) {
            absolute_time_t cur_time = get_absolute_time();
            manager->timers[index].elapsed_time = (double)(to_us_since_boot(cur_time) - to_us_since_boot(manager->timers[index].start_time)); 
            manager->timers[index].active = 0;
        }
    } else {
        printf("Invalid timer index: %d\n", index);
    }
}

void timer_print(TimerManager* manager) {
    printf("Timers:\n");
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (manager->timers[i].elapsed_time > 0) {
            printf("  Timer %d (%s): %f microseconds\n", i, manager->timer_names[i], manager->timers[i].elapsed_time);
        }
    }
}

void timer_print_fps(TimerManager* manager, int index) {
    if (index >= 0 && index < MAX_TIMERS) {
        if (manager->timers[index].elapsed_time > 0) {
            double fps = 1000000.0 / manager->timers[index].elapsed_time;
            printf("  Timer %d (%s) FPS: %.2f\n", index, manager->timer_names[index], fps);
        } else {
            printf("Timer %d (%s) has not been stopped yet or no time recorded!\n", index, manager->timer_names[index]);
        }
    } else {
        printf("Invalid timer index: %d\n", index);
    }
}

void timer_reset(TimerManager* manager) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        manager->timers[i].start_time = nil_time;
        manager->timers[i].elapsed_time = 0;
        manager->timers[i].active = 0;
    }
}
