#include "Data_Handler.h" 
#include "Particle.h" 

enum eventsId { btn1, btn2, btn3, cap_turn, col_off, dispense, curr_time }; 
struct Event {
    int id; 
    char name[10]; 
    unsigned long value; 
}; 

void sendData(String event, String value) {
    
}

void storeData(Event event) { 
    unsigned long encoded_value = encoded(event.value, event.id); 
    
    if(eventsCounter < eventsLimit) { 
        eventsEncoded[eventsCounter] = encoded_value; 
        eventsCounter++; 
    } 
    
    
    int currentAddress; 
    EEPROM.get(0, currentAddress); 
    if(currentAddress == 0xFFFFFFFF) {
        EEPROM.put(0, 8); 
        currentAddress = 8; 
    }
    
    if(( event.id == btn1 || 
            event.id == btn2 || 
            event.id == btn3 || 
            event.id == col_off || 
            event.id == dispense ) && 
        currentAddress < EEPROM_length - 8) {
        EEPROM.put(currentAddress, encoded_value); 
        currentAddress += 8; 
        EEPROM.put(0, currentAddress); 
    }
    
} 

unsigned long encoded(unsigned long value, int code) { 
    long mask = ~0u << 4; 
    value &= mask;
    value |= code; 
    
    return value; 	
}



