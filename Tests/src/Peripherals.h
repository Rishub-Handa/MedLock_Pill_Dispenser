#ifndef PERIPHERALS_H
#define PERIPHERALS_H 

#include "Particle.h"
#include "Main.h"
#include "Data_Handler.h"
#include "Async_Handler.h"
#include "neopixel/neopixel.h"

extern Adafruit_NeoPixel strip; 

struct Input {
    int event_id; 
    int pin; 
    char name[10]; 
    bool activated; 
    unsigned long curr_value; 
}; 

extern Input charger; 

extern int inr_emit;
extern int photo_vcc;
extern Input photo_out;

extern Input btn1;
extern Input btn2;
extern Input btn3;
extern Input collar_btn;
extern Input twist_btn;
extern Input btns[];
extern const int INPUT_COUNT; 

extern bool credentials_correct; 

void check_btns(Input btns[], int size, int mode); 
int special_functions(); 
void check_dispense(); 
void check_infrared_pulses(); 

void handle_user_event(Input btn); 
void handle_setup_event(Input btn); 

void flash_color(int r, int g, int b, int flash_count); 
void flash_error(int r, int g, int b, int flash_count); 
void display_color(int r, int g, int b); 
void data_transmission_lights(int stage); 

void listen_isr(); 

void wakeup_lights(); 
void listen_mode_lights(); 

int pq_flash_on(); 
int pq_flash_off(); 

void clear_color(); 
void display_num(int num); 
void display_doses_left(int dose); 
void display_code(bool a, bool b, bool c, bool d); 


#endif