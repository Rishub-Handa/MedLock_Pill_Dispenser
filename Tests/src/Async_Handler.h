#ifndef ASYNC_HANDLER_H
#define ASYNC_HANDLER_H 

#include "Particle.h"
#include "Main.h"

enum timer_ids { standby_timer_id, pq_on_id, pq_off_id }; 
enum pq_event_ids { pq_collar_id, pq_battery_id, pq_setup_id, pq_charger_id }; 

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

struct PQ_Event {
    int pq_event_id; 
    int priority; 
    int r; 
    int g; 
    int b; 
    bool queued; 
}; 

extern CB_Timer standby_timer; 
extern CB_Timer pq_on_timer; 
extern CB_Timer pq_off_timer; 

extern CB_Timer timers[]; 
extern const int TIMER_COUNT; 

extern PQ_Event pq_collar; 
extern PQ_Event pq_battery; 
extern PQ_Event pq_setup; 
extern PQ_Event pq_charger; 

extern PQ_Event pq_events[]; 
extern const int PQ_EVENT_COUNT; 

// void check_async(CB_Timer async_timers[], int count); 
void check_async(); 






#endif