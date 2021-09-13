#ifndef MAIN_H
#define MAIN_H 

#include "Particle.h"
#include "Connection.h" 
#include "Data_Handler.h" 
#include "Peripherals.h"
#include "Async_Handler.h"
#include "neopixel/neopixel.h" 

enum mode_id { user_mode, setup_mode, admin_mode }; 

extern u_int64_t current_time; 

// FUNCTION DEFINITIONS: 
void reset_standby_timer(); 

// Test Functions 
int test_funcptr(int num); 
void check_mode(); 
int put_to_sleep(); 
// int setup_flash_on(); 
// int setup_flash_off(); 

#endif