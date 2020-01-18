#include "Main.h"
#include "Connection.h" 
#include "Data_Handler.h" 
#include "Peripherals.h"
#include "Async_Handler.h"
#include "neopixel/neopixel.h"
#include "Particle.h" 
#include "string.h" 
#include "softap_http.h" 

int curr_mode = user_mode; 
u_int64_t current_time; 
int queued_dispenses = 0; 
bool has_twist_interrupt = false; 

// DATA STORAGE 

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); 

// WIFI 
STARTUP(setUpInternet()); // Press SETUP for 3 seconds to make the Photon enter Listening mode. Navigate to http://192.168.0.1 to setup Wi-Fi 
SYSTEM_MODE(SEMI_AUTOMATIC); 

// FUNCTION DEFINITIONS: 
void set_pins(); 
void dispense_isr(); 

void setup() {
    set_pins(); 

    u_int64_t curr_time; 
    EEPROM.get(26, curr_time); 
    current_time = curr_time; 

    timers[standby_timer_id].start = millis(); 
    timers[standby_timer_id].activated = true;  

    digitalWrite(inr_emit, HIGH); 
    digitalWrite(photo_vcc, HIGH); 

    check_mode(); 

    attachInterrupt(twist_btn.pin, dispense_isr, RISING); 

    strip.begin(); 
    strip.clear(); 
    strip.show();
    wakeup_lights(); 

    Serial.println("Setup. "); 

} 

void loop() {    
    
    if(digitalRead(A3) == HIGH) {
        Serial.println("High. "); 
    }

    // Improve 
    if(credentials_correct) {
        credentials_correct = false; 
        if(WiFi.ready())  {
            flash_color(0, 20, 0, 5); 
            detachInterrupt(btn2.pin); 
            System.reset(); 
        } else {
            flash_color(20, 0, 0, 5);
        }
        timers[standby_timer_id].activated = true; 
    }

    // Tested 

    check_btns(btns, 5, curr_mode); 

    if(curr_mode == user_mode) {
        if(!has_twist_interrupt) { 
            attachInterrupt(twist_btn.pin, dispense_isr, RISING); 
            has_twist_interrupt = true; 
        }
        // Include Here ? 
        if(queued_dispenses == 0) {
            check_dispense(); 
        } else { 
            while(queued_dispenses > 0) {
                handle_dispense(); 
                queued_dispenses -= 1; 
                Serial.print("Queued Dispenses Left: "); Serial.println(queued_dispenses); 
            } 
        }
            

    } else if(curr_mode == setup_mode) {
        // Check Other Functions 
        if(has_twist_interrupt) {
            detachInterrupt(twist_btn.pin); 
            has_twist_interrupt = false; 
        }
    }

    check_async(); 

    // Compartmentalize 
    if(client.available()) {
        char c = client.read(); 
        Serial.print(c); 
        reset_standby_timer(); 
    }

    delay(100); 
} 

void check_mode() {
    char get_id[24]; 
    EEPROM.get(2, get_id); 
    get_id[25] = 0; 

    if(get_id[0] == 0) {
        curr_mode = setup_mode; 
        Serial.println("Queued PQ Setup. "); 
        pq_events[pq_setup_id].queued = true; 

    } else {
        curr_mode = user_mode; 
        pq_events[pq_setup_id].queued = false; 
    }

} 

int put_to_sleep() { 

    if(digitalRead(collar_btn.pin)) {
        timers[standby_timer_id].start = millis(); 
        timers[standby_timer_id].activated = true;  
        return 2; 
    }

    if(!WiFi.connecting() && !digitalRead(collar_btn.pin)) {
        strip.clear(); 
        strip.show(); 


        // If in user mode 
        if(has_dispense_data() && curr_mode == user_mode) send_data(""); 

        while(queued_dispenses > 0) {
            handle_dispense(); 
            queued_dispenses -= 1; 
            Serial.print("Queued Dispenses Left: "); Serial.println(queued_dispenses); 
        } 

        // Particle.publish(String(analogRead(charge_reading))); 

        System.sleep(SLEEP_MODE_DEEP); 
        return 1; 

    }

    return 0; 
    
}

void dispense_isr() {
    detachInterrupt(twist_btn.pin); 
    Serial.println("Dispense ISR Begin. "); 

    if(curr_mode == user_mode) {
        for(int i = 0; i < 150; i++) {
            Serial.println("ISR Loop. "); 
            if(analogRead(photo_out.pin) < 1800 && !photo_out.activated) { 
                queued_dispenses++; 
                Serial.print("Queued Dispenses: "); Serial.println(queued_dispenses); 
                break; 
            } 
            delayMicroseconds(20000); 
        }
    }

    Serial.println("Dispense ISR End. "); 

    attachInterrupt(twist_btn.pin, dispense_isr, RISING); 
} 

void reset_standby_timer() {
    timers[standby_timer_id].start = millis(); 
    timers[standby_timer_id].activated = true; 
}


void set_pins() {

    pinMode(charger.pin, INPUT); 

    pinMode(inr_emit, OUTPUT); 
    pinMode(photo_vcc, OUTPUT); 
    pinMode(photo_out.pin, INPUT); 

    pinMode(btn1.pin, INPUT); 
    pinMode(btn2.pin, INPUT);  
    pinMode(btn3.pin, INPUT); 

    pinMode(collar_btn.pin, INPUT); 
    pinMode(twist_btn.pin, INPUT_PULLUP); 
    // pinMode(A3, INPUT_PULLUP); 

    pinMode(charge_reading, INPUT); 

}

