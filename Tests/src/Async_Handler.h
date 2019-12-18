#ifndef ASYNC_HANDLER_H
#define ASYNC_HANDLER_H 

#include "Particle.h"
#include "Main.h"

enum timer_ids { standby_timer_id, setup_on_id, setup_off_id }; 

// Callback Timers 
// struct CB_Timer {
//     long start; 
//     int delay; 
//     int (*callback)(); 
//     // bool activated; 
//     // int priority; 
//     // int timer_id; 
// }; 

struct CB_Timer {
    long start; 
    int delay; 
    int (*callback)(); 
    int timer_id; 
    bool activated; 
}; 

extern CB_Timer standby_timer; 
extern CB_Timer setup_on_timer; 
extern CB_Timer setup_off_timer; 

// extern CB_Timer timers[10]; 
// extern int timer_count; 

extern CB_Timer timers[]; 
extern const int TIMER_COUNT; 

// void check_async(CB_Timer async_timers[], int count); 
void check_async(); 






#endif