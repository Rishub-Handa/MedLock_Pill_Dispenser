#include "Data_Handler.h" 
#include "Particle.h" 

const int EVENTS_LIMIT = 300; 
retained unsigned long events_encoded[EVENTS_LIMIT]; 
retained int events_counter = 0; 
const size_t EEPROM_LENGTH = EEPROM.length(); 

// retained int x; 

// int y = 25; 

void testStorage(long value) {
    events_encoded[events_counter] = value; 
    events_counter++; 
    printEvents(); 
} 

void printEvents() {
    Serial.print("[ "); 
    for(int i = 0; i < events_counter; i++) {
        Serial.print(events_encoded[i]); 
        Serial.print(", "); 
    } 
    Serial.println("]. "); 
}


