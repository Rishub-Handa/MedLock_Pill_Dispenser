#include "Connection.h" 
#include "Data_Handler.h" 
#include "Particle.h" 
#include "softap_http.h" 

// TESTED PINS 
int inr_emit = DAC;
int photo_vcc = D0;
int photo_out = A0; 

int btn1 = D5;
int btn2 = D6;
int btn3 = D7; 

// DATA STORAGE 
int eventsCounter = 0; 
const int eventsLimit = 300; 
retained unsigned long eventsEncoded[eventsLimit]; 

size_t EEPROM_length = EEPROM.length(); // The first address contains the index of the current address, which is a multiple of 8. 

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); 

// WIFI 
STARTUP(setUpInternet()); // Press SETUP for 3 seconds to make the Photon enter Listening mode. Navigate to http://192.168.0.1 to setup Wi-Fi 

// FUNCTION DEFINITIONS: 
void set_pins(); 

void setup() {
    set_pins(); 

    if(digitalRead(btn1) == HIGH) {
        Serial.println("Button 1 Pressed. "); 
        int x = 3; 
        EEPROM.put(0, x); 
        delay(1000); 
    } 

    if(digitalRead(btn2) == HIGH) {
        Serial.println("Button 2 Pressed. "); 
        int x; 
        EEPROM.get(0, x); 
        Serial.println(x); 
        delay(1000); 

    }

} 

void loop() { 
    
}

void set_pins() {
    pinMode(inr_emit, OUTPUT); 
    pinMode(photo_vcc, OUTPUT); 
    pinMode(photo_out, INPUT); 

    pinMode(btn1, INPUT); 
    pinMode(btn2, INPUT);  
    pinMode(btn3, INPUT);
}

