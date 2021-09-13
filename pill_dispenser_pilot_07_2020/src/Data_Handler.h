#ifndef DATA_HANDLER_H 
#define DATA_HANDLER_H 

#include "Particle.h" 

enum event_id { btn1_id, btn2_id, btn3_id, cap_turn_id, col_off_id, dispense_id, curr_time_id, charger_id }; 

extern const int CODE_LENGTH; 
extern int code[];
extern int code_counter; 


void send_data(String json_data); 
void send_code(int code[], int length); 
void store_data(int id, unsigned long value); 
void store_code(int button_id); 

bool has_dispense_data(); 

extern TCPClient client; 

// Should Not Be in Header File During Deployment Phase 
void clear_EEPROM(); 
void clear_usage_EEPROM(); 
void clear_SRAM(); 

void test_storage(long value); 
void print_events(); 

void print_uint64_t(uint64_t num); 
String uint64_t_to_String(uint64_t num); 


#endif 