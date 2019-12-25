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

// DATA STORAGE 

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); 

// WIFI 
STARTUP(setUpInternet()); // Press SETUP for 3 seconds to make the Photon enter Listening mode. Navigate to http://192.168.0.1 to setup Wi-Fi 
SYSTEM_MODE(SEMI_AUTOMATIC); 

// FUNCTION DEFINITIONS: 
void set_pins(); 

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

    strip.begin(); 
    strip.clear(); 
    strip.show();
    wakeup_lights(); 

    Serial.println("Setup. "); 

} 

void loop() { 

    if(credentials_correct) {
        credentials_correct = false; 
        if(WiFi.ready())  {
            flash_color(0, 20, 0, 5); 
            detachInterrupt(D6); 
            System.reset(); 
        } else {
            flash_color(20, 0, 0, 5);
        }
        timers[standby_timer_id].activated = true; 
    }

    // Tested

    check_btns(btns, 5, curr_mode); 

    if(curr_mode == user_mode) {
        
        check_dispense(); 
        // check_infrared_pulses(); 

    } else if(curr_mode == setup_mode) {
        // Check Other Functions 
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

    if(!WiFi.connecting() && !digitalRead(collar_btn.pin)) {
        strip.clear(); 
        strip.show(); 

        System.sleep(SLEEP_MODE_DEEP); 
        return 1; 

    }

    return 0; 

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
    pinMode(twist_btn.pin, INPUT); 

}

