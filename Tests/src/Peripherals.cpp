#include "Peripherals.h" 
#include "Particle.h" 
#include "Async_Handler.h"
#include "Data_Handler.h"
#include "neopixel/neopixel.h"

Input charger = { charger_id, D0, "Charger", false, 0 }; 

int inr_emit = DAC;
int photo_vcc = A5;
Input photo_out = { dispense_id, A0, "Dispensed", false, 0 }; 

int charge_reading = A1; 

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

const int MAXPULSE = 10001; 
const int RESOLUTION = 5; 
uint16_t pulses[100][2]; 
uint8_t currentpulse = 0; 

bool credentials_correct = false; 

// Auxiliary Functions 
int get_priority_index(); 
// Unsafe function 
void check_if_dispense(); 
void printpulses(); 

void check_btns(Input btns[], int size, int mode) {

    if(special_functions()) return; 

    // Charger logic flipped. 1 when not charging, 0 when charging. 
    if(!digitalRead(charger.pin) && !charger.activated) {
        flash_color(25, 16, 0, 3); 
        charger.activated = true; 
    } 

    if(digitalRead(charger.pin) && charger.activated) {
        charger.activated = false; 
    } 



    // Measure the Threshold 
    // if(analogRead(charge_reading) < 200) {
    //     pq_events[pq_battery_id].queued = true; 
    // } else {
    //     pq_events[pq_battery_id].queued = false; 
    // }

    for(int i = 0; i < size; i++) {
        if(digitalRead(btns[i].pin) == HIGH && !btns[i].activated){ 
            btns[i].activated = true; 
            reset_standby_timer(); 

            // Particle.publish("Charge Reading", String(analogRead(charge_reading)), PRIVATE);

            if(mode == setup_mode) {
                handle_setup_event(btns[i]); 
            } else if(mode == user_mode && btns[i].event_id == col_off_id) {
                handle_user_event(btns[i]); 
            }

            // if(btns[i].event_id == charger_id && mode == user_mode) {
            //     Serial.println("Charger Activated"); 
            //     send_data(""); 
            //     pq_events[pq_charger_id].queued = true; 
            // } 

            if(btns[i].event_id == col_off_id) {
                pq_events[pq_collar_id].queued = true; 
                reset_standby_timer(); 
            }


            Serial.print(btns[i].name);  Serial.println(" Pressed. "); 
            Serial.print("Real Time Clock: "); Serial.println(Time.now()); 
        }
    
        if(digitalRead(btns[i].pin) == LOW && btns[i].activated) {
            btns[i].activated = false; 
            reset_standby_timer(); 

            if(mode == user_mode && btns[i].event_id != col_off_id) {
                handle_user_event(btns[i]); 
            } else if(mode == setup_mode) {
                clear_color(); 
            }

            if(btns[i].event_id == col_off_id) {
                pq_events[pq_collar_id].queued = false; 
            } 

            // if(btns[i].event_id == charger_id) {
            //     pq_events[pq_charger_id].queued = false; 
            // }

            Serial.print(btns[i].name);  Serial.println(" Released. "); 
        }
    }
} 

int check_dispense() {
    if(analogRead(photo_out.pin) < 1800 && !photo_out.activated) { 
        handle_dispense(); 
        return 1; 
    } 

    if(analogRead(photo_out.pin) >= 1800 && photo_out.activated) {
        photo_out.activated = false; 
        return 2; 
    }

    return 0; 
}

void check_infrared_pulses() {
    uint16_t highpulse, lowpulse; 
    highpulse = lowpulse = 0; 
 
 
    while (digitalRead(10)) { 
        highpulse++;
        delayMicroseconds(RESOLUTION);
        if ((highpulse >= MAXPULSE) && (currentpulse != 0)) {
            printpulses();
            currentpulse = 0;
            return;
        }
    }
    pulses[currentpulse][0] = highpulse;

    while (!digitalRead(10)) {
        lowpulse++;
        delayMicroseconds(RESOLUTION);
        if ((lowpulse >= MAXPULSE)  && (currentpulse != 0)) {
            printpulses();
            currentpulse = 0;
        return;
        }
    }
    pulses[currentpulse][1] = lowpulse;

    currentpulse++;
}

void printpulses(void) {
    Serial.println("\n\r\n\rReceived: \n\rOFF \tON");
    for (uint8_t i = 0; i < currentpulse; i++) {
        Serial.print(pulses[i][0] * RESOLUTION, DEC);
        Serial.print(" usec, ");
        Serial.print(pulses[i][1] * RESOLUTION, DEC);
        Serial.println(" usec");
    }
}

void handle_dispense() {
    Serial.println("Handling Dispense. "); 
    photo_out.activated = true; 
    reset_standby_timer(); 

    u_int32_t time_now = Time.now(); 
    u_int32_t stored_curr_time_converted = (u_int32_t)(current_time / 1000); 
    long time_difference = time_now - stored_curr_time_converted; 
    u_int32_t value; 

    if(time_difference < 0) 
        value = 0; 
    else value = time_difference; 

    store_data(photo_out.event_id, value); 
    flash_color(0, 0, 20, 5); 

    Serial.println("Dispense. "); 
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
        case col_off_id: 
            break; 
        case cap_turn_id: 
            // display_color(10, 20, 0); 
            break; 
        default: 
            break; 
    }
}


void handle_user_event(Input btn) {

    u_int32_t time_now = Time.now(); 
    u_int32_t stored_curr_time_converted = (u_int32_t)(current_time / 1000); 
    long time_difference = time_now - stored_curr_time_converted; 
    u_int32_t value; 
    if(time_difference < 0) 
        value = 0; 
    else value = time_difference; 

    Serial.print("Real Time Clock: "); Serial.println(time_now); 
    Serial.print("Current Time Raw: "); print_uint64_t(current_time); 
    Serial.print("Current Time Conversion: "); Serial.println(stored_curr_time_converted);
    Serial.print("Logged Time: "); Serial.println(time_difference); 

    store_data(btn.event_id, value); 

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
            break; 
        case cap_turn_id: 
            // flash_color(10, 20, 0, 3); 
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

    // Timer 
    // Early timeout due to standby timer 
    // Should timeout after 30 seconds if not correct. Interrupt to reset. 
    // If correct, should timeout soon. 
    // After restarting, timeout should be 20 seconds. 
    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {

        int timer_count = 0; 

        while(digitalRead(btn1.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
            timer_count++; 
            delay(500); 
            if(timer_count > 10) break; 
        }

        if(timer_count > 10) {
            timers[standby_timer_id].delay = 120000; 
            credentials_correct = true; 
            listen_mode_lights(); 
            attachInterrupt(btn2.pin, listen_isr, RISING);  
            // WiFi.setListenTimeout(120); 
            WiFi.listen(); 
            return 1; 
        } else return 0; 

    }

    if(digitalRead(btn2.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {

        int timer_count = 0; 

        while(digitalRead(btn2.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
            timer_count++; 
            delay(500); 
            if(timer_count > 10) break; 
        }

        if(timer_count > 10) {
            timers[standby_timer_id].delay = 120000; 
            flash_color(0, 0, 20, 2); 
            Particle.connect(); 
            return 1; 
        } else return 0; 

    }
    return 0; 
} 

void listen_isr() {
    Serial.println("Listen ISR. "); 
    detachInterrupt(btn2.pin); 
    System.reset(); 
}

void flash_color(int r, int g, int b, int flash_count) {

    for(int i = 0; i < flash_count; i++) {

        for(int j = 0; j < 4; j++) 
            strip.setPixelColor(j, r, g, b); 
        strip.show();
        
        // long curr_time = millis(); 
        // while(millis() < curr_time + 250) { }
        delayMicroseconds(250000); 
        strip.clear(); strip.show();
        // curr_time = millis(); 
        // while(millis() < curr_time + 250) { }
        delayMicroseconds(250000); 
    }

}  

void flash_error(int r, int g, int b, int flash_count) {
    for(int i = 0; i < flash_count; i++) {

        for(int j = 0; j < 3; j++) 
            strip.setPixelColor(j, 20, 0, 0); 

        strip.setPixelColor(3, r, g, b); strip.show();

        long curr_time = millis(); 
        while(millis() < curr_time + 250) { }
        strip.clear(); strip.show();
        curr_time = millis(); 
        while(millis() < curr_time + 250) { }
    }
}

int pq_flash_on() {

    timers[pq_on_id].activated = false; 
    timers[pq_off_id].start = millis(); 
    timers[pq_off_id].activated = true; 

    int priority_index = get_priority_index(); 

    if(priority_index == -1) {
        strip.clear(); 
        strip.show(); 

        timers[pq_on_id].activated = false; 
        timers[pq_off_id].activated = false; 

        return 0; 
    }


    int r = pq_events[priority_index].r; 
    int g = pq_events[priority_index].g; 
    int b = pq_events[priority_index].b; 

    // For Setup Only 
    if(priority_index == 2) {
        for(int j = 0; j < INPUT_COUNT; j++) {
            if(btns[j].activated == true) {
                return 0; 
            }
        }
    }

    // Change number of LEDs to 1 for Low Battery Indicator 

    int num_LEDs = 4; 

    // Test 
    // if(priority_index == 1) {
    //     num_LEDs = 2; 
    // }

    for(int i = 0; i < num_LEDs; i++) 
        strip.setPixelColor(i, r, g, b); 

    strip.show();
    return 1; 
}

int pq_flash_off() {

    timers[pq_off_id].activated = false; 
    timers[pq_on_id].start = millis(); 
    timers[pq_on_id].activated = true; 

    int priority_index = get_priority_index(); 

    if(priority_index == -1) {
        strip.clear(); 
        strip.show(); 

        timers[pq_on_id].activated = false; 
        timers[pq_off_id].activated = false; 

        return 0; 
    }

    // For Setup Only 
    if(priority_index == 2) {
        for(int j = 0; j < INPUT_COUNT; j++) {
            if(btns[j].activated == true) {
                return 0; 
            }
        }
    } 


    strip.clear(); 

    // Test 
    // if(priority_index == 1) {
    //     strip.setPixelColor(0, 20, 0, 0); 
    // }

    strip.show(); 


    return 1; 
}

int get_priority_index() {
    int priority = PQ_EVENT_COUNT + 1; 
    int priority_index = -1; 
    for(int i = 0; i < PQ_EVENT_COUNT; i++) {
        if(pq_events[i].queued && pq_events[i].priority < priority) {
            priority = pq_events[i].priority; 
            priority_index = i; 
        }
    } 
    return priority_index; 
}

void data_transmission_lights(int stage) {
    if(stage >= 1 && stage <= 4) {
        for(int i = 0; i < stage; i++) {
            strip.setPixelColor(i, 0, 19, 26); 
        }
        strip.show(); 
    }
    if(stage == 5) {
        flash_color(0, 19, 26, 5); 
    }
} 

void wakeup_lights() {
    strip.clear(); 
    strip.show(); 
    for(int i = 0; i < 4; i++) {
        strip.setPixelColor(i, 0, 0, 20); 
        strip.show(); 
        delay(200); 
    } 
    strip.clear(); 
    strip.show(); 
} 

void low_battery_lights() {
    strip.clear(); 
    strip.show(); 

    for(int i = 0; i < 3; i++) {
        strip.setPixelColor(3, 20, 0, 0); 
        strip.setPixelColor(2, 20, 0, 0); 
        strip.show(); 
        delay(250); 
        strip.clear(); 
        strip.setPixelColor(3, 20, 0, 0); 
        strip.show(); 
        delay(250); 
    }

}


void listen_mode_lights() {
    strip.clear(); 
    strip.show(); 
    for(int i = 0; i < 4; i++) {
        strip.setPixelColor(i, 20, 20, 0); 
        strip.show(); 
        delay(100); 
    } 
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
