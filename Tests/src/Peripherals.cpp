#include "Peripherals.h" 
#include "Particle.h" 
#include "Async_Handler.h"
#include "Data_Handler.h"
#include "neopixel/neopixel.h"

Input charger = { charger_id, D4, "Charger", false, 0 }; 

int inr_emit = DAC;
int photo_vcc = D0;
Input photo_out = { dispense_id, A0, "Dispensed", false, 0 }; 

Input btn1 = { btn1_id, D5, "Button 1", false, 0 }; 
Input btn2 = { btn2_id, D6, "Button 2", false, 0 }; 
Input btn3 = { btn3_id, D7, "Button 3", false, 0 }; 
Input collar_btn = { col_off_id, A4, "Col Off", false, 0}; 
Input twist_btn = { cap_turn_id, A3, "Cap Turn", false, 0}; 
// Input btns[] = { btn1, btn2, btn3, collar_btn, twist_btn, charger }; 
Input btns[] = { btn1, btn2, btn3, collar_btn, twist_btn }; 
const int INPUT_COUNT = 5; 


const int LED_PIN = D3; 
const int LED_COUNT = 4; 
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, LED_PIN); 


// TODO: Special Functions and Admin Mode 
// If multiple events interrupt each other, affects activated field 
void check_btns(Input btns[], int size, int mode) {

    if(special_functions()) return; 

    for(int i = 0; i < size; i++) {
        if(digitalRead(btns[i].pin) == HIGH && !btns[i].activated){ 
            btns[i].activated = true; 
            reset_standby_timer(); 

            if(mode == setup_mode) {
                handle_setup_event(btns[i]); 
            } 

            if(btns[i].event_id == charger_id && mode == user_mode) {
                Serial.println("Charger Activated"); 
                send_data(""); 
                flash_color(25, 16, 0, 5); 
            }


            Serial.print(btns[i].name);  Serial.println(" Pressed. "); 
        }
    
        if(digitalRead(btns[i].pin) == LOW && btns[i].activated) {
            btns[i].activated = false; 
            reset_standby_timer(); 

            if(mode == user_mode) {
                handle_user_event(btns[i]); 
            } else if(mode == setup_mode) {
                clear_color(); 
            }

            Serial.print(btns[i].name);  Serial.println(" Released. "); 
        }
    }
} 

void check_dispense() {
    if(analogRead(photo_out.pin) < 20 && !photo_out.activated) { 
        photo_out.activated = true; 
        reset_standby_timer(); 
        store_data(photo_out.event_id, millis());  

        flash_color(0, 0, 20, 5); 

        Serial.println("Dispense. "); 
    } 

    if(analogRead(photo_out.pin) >= 20 && photo_out.activated) {
        photo_out.activated = false; 
    }
}


void handle_setup_event(Input btn) {
    switch(btn.event_id) {
        case btn1_id: 
            store_code(btn.event_id); 
            display_color(0, 20, 0); 
            break; 
        case btn2_id: 
            store_code(btn.event_id); 
            display_color(20, 20, 0); 
            break; 
        case btn3_id: 
            store_code(btn.event_id); 
            display_color(20, 0, 0);
            break; 
        // Design Decision on what to do if collar opened 
        case col_off_id: 
            // store_data(btn.event_id, millis()); 
            display_color(20, 10, 0); 
            break; 
        case cap_turn_id: 
            // store_data(btn.event_id, millis()); 
            display_color(10, 20, 0); 
            break; 
        default: 
            break; 
    }
}


// Test 
void handle_user_event(Input btn) {
    store_data(btn.event_id, millis()); 

    switch(btn.event_id) {
        case btn1_id: 
            flash_color(0, 20, 0, 3); 
            break; 
        case btn2_id: 
            flash_color(20, 20, 0, 3); 
            break; 
        case btn3_id: 
            flash_color(20, 0, 0, 3); 
            break; 
        case col_off_id: 
            flash_color(20, 10, 0, 3); 
            break; 
        case cap_turn_id: 
            flash_color(10, 20, 0, 3); 
            break; 
        default: 
            break; 
    }

}


int special_functions() {

    // Should not record these events in storage 

    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn2.pin) == HIGH) {
        btn1.activated = false; 
        btn2.activated = false; 
        clear_EEPROM(); 
        clear_SRAM(); 
        return 1; 
    } 

    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
        send_data(""); 
        delay(1000); 
        return 1; 

    }

    if(digitalRead(btn2.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
        System.sleep(SLEEP_MODE_DEEP); 
        return 1; 
    } 

    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn2.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
        // Setup Mode 
    }

    return 0; 
}






void flash_color(int r, int g, int b, int flash_count) {

    for(int i = 0; i < flash_count; i++) {

        for(int j = 0; j < 4; j++) 
            strip.setPixelColor(j, r, g, b); 

        strip.show();
        // Alternative to delay ??? 
        // delay(500);
        long curr_time = millis(); 
        // for(int i = 0; i < curr_time + 500; i++) { } 


        while(millis() < curr_time + 250) {
            
        }
        strip.clear();
        strip.show();
        curr_time = millis(); 
        while(millis() < curr_time + 250) {
            
        }
    }

}  





// How to externally break this loop ??? 
int setup_flash_on() {

    timers[setup_on_id].activated = false; 
    timers[setup_off_id].start = millis(); 
    timers[setup_off_id].activated = true; 

    // CB_Timer new_timer = { millis(), 250, setup_flash_off, setup_off_id, true }; 
    // timers[timer_count - 1] = new_timer; 

    for(int j = 0; j < INPUT_COUNT; j++) {
        if(btns[j].activated == true) {
            return 0; 
        }
    }

    for(int i = 0; i < 4; i++) 
        strip.setPixelColor(i, 25, 18, 19); 

    strip.show();
    return 1; 
}

int setup_flash_off() {

    timers[setup_off_id].activated = false; 
    timers[setup_on_id].start = millis(); 
    timers[setup_on_id].activated = true; 

    // CB_Timer new_timer = { millis(), 250, setup_flash_on, setup_on_id, true }; 
    // timers[timer_count - 1] = new_timer; 

    for(int j = 0; j < INPUT_COUNT; j++) {
        if(btns[j].activated == true) {
            return 0; 
        }
    }

    strip.clear(); 
    strip.show(); 


    return 1; 
}










void display_color(int r, int g, int b) {
    for(int j = 0; j < 4; j++) 
            strip.setPixelColor(j, r, g, b); 
    strip.show();

}
void clear_color() {
    strip.clear(); 
    strip.show(); 
}
