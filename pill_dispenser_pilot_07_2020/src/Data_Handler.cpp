#include "Data_Handler.h" 
#include "Main.h"
#include "Connection.h" 
#include "Peripherals.h" 
#include "Particle.h" 
#include "string.h" 
#include "Async_Handler.h"

const int EVENTS_LIMIT = 300; 
retained u_int32_t events_encoded[EVENTS_LIMIT]; 
retained int events_counter = 0; 
const size_t EEPROM_LENGTH = EEPROM.length(); 

TCPClient client; 

const int CODE_LENGTH = 6; 
int code[CODE_LENGTH];
int code_counter = 0; 

u_int32_t encoded(unsigned long value, int code); 
String decode_name(unsigned long value); 
String format_json(); 
void handle_server_res(String res); 
void print_uint64_t(uint64_t num); 
String uint64_t_to_String(uint64_t num); 

void store_code(int btn_id) {

    if(code_counter < CODE_LENGTH) {
        code[code_counter] = btn_id; 
        code_counter++; 
    } 
    
    if(code_counter >= CODE_LENGTH) { 
        
        send_code(code, CODE_LENGTH); 

        Serial.println("Send code. "); 
        for(int i = 0; i < CODE_LENGTH; i++) {
            Serial.print(String(code[i])); 
            Serial.print(", "); 
        } 
        Serial.println(); 
    }
    
}

void send_code(int code[], int length) {
    String json_data = "{ \"code\": [ "; 

    for(int i = 0; i < length; i++) {
        json_data += String(code[i]); 
        if(i < length - 1) {
            json_data += ","; 
        }
    }

    json_data += "] }"; 
    Serial.println(json_data); 

    send_data(json_data); 

}


void send_data(String json_data) { 

    int CONNECTION_TRIES = 1; 
    
    // If charging, CONNECTION_TRIES = 3; 
    if(pq_events[pq_charger_id].queued) {
        CONNECTION_TRIES = 3; 
    }


    for(int i = 0; i < CONNECTION_TRIES; i++) {
        if(!WiFi.ready()) {
            if(!connectToInternet()) {
                Serial.println("Connection Attempt Failure. "); 
                if(i == CONNECTION_TRIES - 1) {
                    flash_error(0, 0, 20, 5); 
                }
                // return; 
            } else {
                data_transmission_lights(2); 
            }
        }
    }

    String json_string = json_data; 
    if(json_string.length() == 0) {
        json_string = format_json(); 
    }
    
    Serial.println(json_string); 

    if (client.connect("f00fd8b3.ngrok.io", 80)) {         
        Serial.println("connected");
                
        client.println("POST /api/dispense HTTP/1.1");
        client.println("Host: f00fd8b3.ngrok.io");
        client.println("Content-Type: application/json");
        client.print("Content-Length: ");
        client.println(strlen(json_string));
        client.println();
        client.println(json_string);
        client.println();

        int wait_count = 0; 
        while(!client.available()) {
            Serial.println("Waiting. "); 
            wait_count++; 
            delay(500); 
            if(wait_count > 20) {
                Serial.println("Server Connection Failed. "); 
                flash_error(26, 14, 0, 5); 
                return; 
            }
        }

        Serial.print("Reading Bytes Available: "); 
        Serial.println(client.available());

        char server_res[512]; 
        int server_res_counter = 0; 

        while(client.available()) {
            char c = client.read();
            server_res[server_res_counter] = c; 
            server_res_counter++; 
            if(server_res_counter > 511) {
                if(String(server_res).indexOf("502 Bad Gateway ") != -1) {
                    Serial.println("Server Connection Failed. 502 Bad Gateway. "); 
                    flash_error(26, 14, 0, 5); 
                    return; 
                }
                break; 
            }
        }

        data_transmission_lights(3); 
        handle_server_res(String(server_res)); 
        client.stop();
    }
    else
    {
        Serial.println("connection failed");
        flash_error(26, 14, 0, 5); 
    }
} 

void handle_server_res(String res) {
    Serial.println(res); 
    
    if(res.indexOf("200 OK") != -1) {
        int dispenser_index; 
        int date_index; 
        Particle.publish("Charge Reading", String(analogRead(charge_reading)), PRIVATE); 
        if((dispenser_index = res.indexOf("Dispenser ID: ")) != -1) {
            int start_index = dispenser_index + String("Dispenser ID: ").length(); 
            String id = res.substring(start_index, start_index + 24); 

            char id_buf[25]; 
            id.toCharArray(id_buf, 25); 
            EEPROM.put(2, id_buf); 

            char get_id[24]; 
            EEPROM.get(2, get_id); 
            Serial.print("Get ID: "); 
            Serial.println(get_id); 
        } 

        if((date_index = res.indexOf("Current Date: ")) != -1) {
            int start_index = date_index + String("Current Date: ").length(); 

            // Check Date Range 
            String date = res.substring(start_index, start_index + 13); 

            u_int64_t date_ms = atoll(date); 
            EEPROM.put(26, date_ms); 

            u_int32_t date_sec = (u_int32_t)(date_ms / 1000); 
            Time.setTime(date_sec); 

            u_int64_t get_date; 
            EEPROM.get(26, get_date); 
            Serial.print("Get Date: "); 
            print_uint64_t(get_date); 
            Serial.println(); 
            current_time = get_date; 

        }
        if(res.indexOf("Wrong Code") != -1) {
            flash_error(20, 20, 0, 5); 
        }
        data_transmission_lights(4); 
        clear_usage_EEPROM(); 
    } else {
        flash_error(26, 14, 0, 5); 
    }
}

void store_data(int id, unsigned long value) {
    u_int32_t encoded_value = encoded(value, id); 
    
    if(events_counter < EVENTS_LIMIT) { 
        events_encoded[events_counter] = encoded_value; 
        events_counter++; 
    } 
    
    u_int16_t curr_address; 
    EEPROM.get(0, curr_address); 
    if(curr_address == -1) {
        EEPROM.put(0, 34ul); 
        curr_address = 34; 
    }
    
    if(( id == btn1_id || 
            id == btn2_id || 
            id == btn3_id || 
            id == col_off_id || 
            id == dispense_id || 
            id == curr_time_id ) && 
        curr_address < EEPROM_LENGTH - 4) {
        EEPROM.put(curr_address, encoded_value); 
        curr_address += 4; 
        EEPROM.put(0, curr_address); 
    } else if(curr_address >= EEPROM_LENGTH - 4) {
        flash_error(14, 4, 23, 5); 
    }
    print_events(); 
}

void test_storage(long value) {
    events_encoded[events_counter] = value; 
    events_counter++; 
    print_events(); 
} 

void print_events() {
    u_int16_t curr_address; 
    EEPROM.get(0, curr_address); 
    if(curr_address != -1) {

        char get_id[24]; 
        EEPROM.get(2, get_id); 

        u_int64_t curr_time; 
        EEPROM.get(26, curr_time); 

        Serial.print("EEPROM "); 
        Serial.print(" | Index "); 
        Serial.print(curr_address); 
        Serial.print(" | ID: "); 
        Serial.print(get_id); 
        Serial.print(" | Current Time: "); 
        print_uint64_t(curr_time); 
        Serial.println(); 

        Serial.print("Data: [ "); 
        for(unsigned int i = 34; i < curr_address; i += 4) { 
            u_int32_t value; 
            EEPROM.get(i, value); 
            Serial.print(value); 
            Serial.print(", "); 
        } 
        Serial.println("]. "); 
    }
} 

bool has_dispense_data() {

    u_int16_t curr_address; 
    EEPROM.get(0, curr_address); 

    if(curr_address >= 34) {
        for(unsigned int i = 34; i < curr_address; i += 4) { 
            u_int32_t value; 
            EEPROM.get(i, value); 
            if(decode_name(value).equals(String("Dispense"))) return true; 
        } 
    }

    return false; 
}

u_int32_t encoded(unsigned long value, int code) { 
    long mask = ~0u << 4; 
    value = value << 4; 
    value &= mask; 
    value |= code; 
    
    return (u_int32_t)value; 	
} 

String decode_name(unsigned long value) {
    long mask = 0xF; 
    unsigned int code = value & mask; 

    switch(code) {
        case btn1_id: 
            return "Button 1"; 
            break; 
        case btn2_id: 
            return "Button 2"; 
            break; 
        case btn3_id: 
            return "Button 3"; 
            break; 
        case cap_turn_id: 
            return "Cap Turn"; 
            break; 
        case col_off_id: 
            return "Col Off"; 
            break; 
        case dispense_id: 
            return "Dispense"; 
            break; 
        case curr_time_id: 
            return "Curr Time"; 
            break; 
        default: 
            return "Default"; 
            break;
    }

}

String format_json() {

    u_int16_t curr_address; 
    EEPROM.get(0, curr_address); 

    char get_id[24]; 
    EEPROM.get(2, get_id); 
    get_id[25] = 0; 
    String id_str = String(1); 

    if(get_id[0] != 0) {
        id_str = String(get_id); 
        id_str = id_str.substring(0, id_str.length()-1); 
    }

    String res = "{ \"id\": \"" + id_str + "\", \"events\": [ "; 

    if(curr_address != -1) {
        for(unsigned int i = 34; i < curr_address; i += 4) { 
            u_int32_t value; 
            EEPROM.get(i, value); 

            res += " { \"name\": "; 
            res += " \"" + decode_name(value) + "\", "; 
            res += " \"value\": "; 
            res += "" + String(value >> 4) + " }"; 
            
            if(i < curr_address - 4) {
                res += ","; 
            }
        } 
        Serial.println("]. "); 
    }

    u_int64_t curr_time; 
    EEPROM.get(26, curr_time); 
    String time_str = String(1); 

    if(curr_time != 0) time_str = uint64_t_to_String(curr_time); 

    res += " ], \"current_time\": " + time_str + " }"; 

    Serial.println("Sending: "); 
    Serial.println(res); 

    return res; 
}

void clear_EEPROM() { 
    EEPROM.clear(); 
    EEPROM.put(0, 34ul); 
    EEPROM.put(2, 0); 
    u_int64_t curr_time = 0; 
    EEPROM.put(26, curr_time); 
    Serial.println("EEPROM cleared. "); 
    code_counter = 0; 
    flash_color(14, 4, 23, 5); 
    check_mode(); 
} 

void clear_usage_EEPROM() {

    Serial.println("Clear Usage EEPROM begin"); 

    char get_id[24]; 
    EEPROM.get(2, get_id); 
    u_int64_t curr_time; 
    EEPROM.get(26, curr_time); 

    EEPROM.clear(); 
    EEPROM.put(0, 34ul); 
    EEPROM.put(2, get_id); 
    EEPROM.put(26, curr_time); 
    code_counter = 0; 

    Serial.println("Clear Usage EEPROM end"); 
    print_events(); 


    data_transmission_lights(5); 
    check_mode(); 
}

void clear_SRAM() {
    memset(events_encoded, 0, sizeof(long) * events_counter); 
    events_counter = 0; 
    Serial.println("SRAM cleared. "); 
}

void print_uint64_t(uint64_t num) {

    char rev[128]; 
    char *p = rev+1;

    while (num > 0) {
        *p++ = '0' + ( num % 10);
        num/= 10;
    }
    p--;
    while (p > rev) {
        Serial.print(*p--);
    }
}

String uint64_t_to_String(uint64_t num) {

    char rev[128]; 
    char *p = rev + 1;
    char str[18]; 


    while (num > 0) {
        *p++ = '0' + ( num % 10);
        num/= 10;
    }
    p--;
    int i = 0; 
    while (p > rev) {
        str[i] = *p--; 
        i++; 
    }

    str[i] = 0; 

    return String(str); 

}
