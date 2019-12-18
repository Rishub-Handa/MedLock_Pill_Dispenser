#include "Particle.h" 
#include "neopixel/neopixel.h"
#include "Async_Handler.h"
#include "Main.h"

CB_Timer standby_timer = { 0, 10000, put_to_sleep, standby_timer_id, false }; 
CB_Timer setup_on_timer = { 0, 250, setup_flash_on, setup_on_id, false }; 
CB_Timer setup_off_timer = { 0, 250, setup_flash_off, setup_off_id, false }; 

CB_Timer timers[] = { standby_timer, setup_on_timer, setup_off_timer }; 
const int TIMER_COUNT = 3; 

// void check_async(CB_Timer async_timers[], int count) {
//     for(int i = 0; i < count; i++) {
//         if(millis() > async_timers[i].start + async_timers[i].delay &&
//             async_timers[i].activated) {
//             int ret_val = async_timers[i].callback(); 
//             // IDEA: int ret_val = async_timers[i].callback(async_timers[i].args); 
//         } 
//     }
// }

void check_async() {
    for(int i = 0; i < TIMER_COUNT; i++) {
        if(timers[i].activated && 
            millis() > timers[i].start + timers[i].delay) {
                int ret_val = timers[i].callback(); 
        }
    }
}