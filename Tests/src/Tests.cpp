/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/rishubhanda/Desktop/MedLock_Dev/Firmware/Tests/src/Tests.ino"
#include "Connection.h" 
#include "Data_Handler.h" 
#include "Particle.h" 
#include "softap_http.h" 

// Status: Everything works currently. 
// Buttons are pressed and logged in SRAM and EEPROM. 
// SRAM and EEPROM can be cleared. 
// SRAM and EEPROM data can both be sent to server depending on which has more data. 
// TODO: 
// Check if server response was successful 
// Acknowledge server success 
// Interrupts 
// Encryption 

void setup();
void loop();
#line 16 "/Users/rishubhanda/Desktop/MedLock_Dev/Firmware/Tests/src/Tests.ino"
struct Input {
    int event_id; 
    int pin; 
    char name[10]; 
    bool activated; 
    unsigned long curr_value; 
}; 

// TESTED PINS 
int inr_emit = DAC;
int photo_vcc = D0;
// int photo_out = A0; 
Input photo_out = { dispense_id, A0, "Dispensed", false, 0 }; 

// int btn1 = D5;
// int btn2 = D6;
// int btn3 = D7; 
Input btn1 = { btn1_id, D5, "Button 1", false, 0 }; 
Input btn2 = { btn2_id, D6, "Button 2", false, 0 }; 
Input btn3 = { btn3_id, D7, "Button 3", false, 0 }; 
Input btns[] = { btn1, btn2, btn3 }; 

// Testing 
int collar_vcc = D2;
int collar_out = A4; 
// Input collar_out = { col_off_id, A4, "Coll Off", false, 0}; 

// DATA STORAGE 

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); 

// WIFI 
STARTUP(setUpInternet()); // Press SETUP for 3 seconds to make the Photon enter Listening mode. Navigate to http://192.168.0.1 to setup Wi-Fi 

// FUNCTION DEFINITIONS: 
void set_pins(); 
void SRAM_test(); 
void check_btns(Input btns[], int size); 
void special_functions(); 

void setup() {
    set_pins(); 

} 

void loop() { 

    check_btns(btns, 3); 

    special_functions(); 

    // Testing 
    Serial.println(analogRead(collar_out)); 


    delay(100); 
} 

void check_btns(Input btns[], int size) {

    for(int i = 0; i < size; i++) {
        if(digitalRead(btns[i].pin) == HIGH && !btns[i].activated){ 
            btns[i].activated = true; 
            store_data(btns[i].event_id, millis()); 

            Serial.print(btns[i].name); 
            Serial.println(" Pressed. "); 
        }
    
        if(digitalRead(btns[i].pin) == LOW && btns[i].activated) {
            btns[i].activated = false; 

            Serial.print(btns[i].name); 
            Serial.println(" Released. "); 
        }
    }
} 

void special_functions() {

    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn2.pin) == HIGH) {
        clear_EEPROM(); 
        clear_SRAM(); 
    } 

    if(digitalRead(btn1.pin) == HIGH && digitalRead(btn3.pin) == HIGH) {
        send_data(); 
        delay(1000); 
    }




}



void set_pins() {
    pinMode(inr_emit, OUTPUT); 
    pinMode(photo_vcc, OUTPUT); 
    pinMode(photo_out.pin, INPUT); 

    pinMode(btn1.pin, INPUT); 
    pinMode(btn2.pin, INPUT);  
    pinMode(btn3.pin, INPUT); 

    // Testing 
    pinMode(collar_vcc, OUTPUT); 
    pinMode(collar_out, INPUT); 
}

