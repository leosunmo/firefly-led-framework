#ifndef SIMPLE_EXEC_TIMER_H
#define SIMPLE_EXEC_TIMER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

// Definitions
#define MAX_TIMERS 10
#define TIMER_NAME_LENGTH 32

// Timer and TimerManager structs
typedef struct {
    absolute_time_t start_time;
    double elapsed_time;
    int active;
} Timer;

typedef struct {
    Timer timers[MAX_TIMERS];
    char timer_names[MAX_TIMERS][TIMER_NAME_LENGTH];
} TimerManager;

// Function declarations
TimerManager* timer_manager_create();
void timer_set_name(TimerManager* manager, int index, const char* name);
void timer_start(TimerManager* manager, int index);
void timer_stop(TimerManager* manager, int index);
void timer_print(TimerManager* manager);
void timer_print_fps(TimerManager* manager, int index);
void timer_reset(TimerManager* manager);

#endif // SIMPLE_EXEC_TIMER_H
