#ifndef DATA_HANDLER_H 
#define DATA_HANDLER_H 

#include "Particle.h" 

extern int eventsCounter; 
const int eventsLimit = 300; 
extern retained unsigned long eventsEncoded[eventsLimit]; 
size_t EEPROM_length = EEPROM.length(); 

void sendData(String event, String value); 
void storeData(Event event); 
unsigned long encoded(unsigned long value, int code); 







#endif 