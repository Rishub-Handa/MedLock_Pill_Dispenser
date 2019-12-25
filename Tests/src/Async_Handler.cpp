#include "Particle.h" 
#include "neopixel/neopixel.h"
#include "Async_Handler.h"
#include "Main.h"

CB_Timer standby_timer = { 0, 20000, put_to_sleep, standby_timer_id, false }; 
CB_Timer pq_on_timer = { 0, 250, pq_flash_on, pq_on_id, false }; 
CB_Timer pq_off_timer = { 0, 250, pq_flash_off, pq_off_id, false }; 

CB_Timer timers[] = { standby_timer, pq_on_timer, pq_off_timer }; 
const int TIMER_COUNT = 3; 

PQ_Event pq_collar = { pq_collar_id, 1, 20, 0, 0, false }; 
PQ_Event pq_battery = { pq_battery_id, 2, 20, 0, 0, false }; 
PQ_Event pq_setup = { pq_setup_id, 3, 26, 11, 18, false }; 
PQ_Event pq_charger = { pq_charger_id, 4, 26, 14, 0, false }; 

extern PQ_Event pq_events[] = { pq_collar, pq_battery, pq_setup, pq_charger }; 
extern const int PQ_EVENT_COUNT = 4; 

void check_async() {
    for(int i = 0; i < TIMER_COUNT; i++) {
        if(timers[i].activated && 
            millis() > timers[i].start + timers[i].delay) {
                int ret_val = timers[i].callback(); 
        }
    }

    for(int i = 0; i < PQ_EVENT_COUNT; i++) {
        if(pq_events[i].queued && !timers[pq_on_id].activated && !timers[pq_off_id].activated) {
            timers[pq_on_id].start = millis(); 
            timers[pq_on_id].activated = true; 
        }
    }
}