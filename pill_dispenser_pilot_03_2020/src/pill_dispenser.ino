/*
 * Project pill_dispenser
 * Description:
 * Author: Rishub Handa 
 * Date:
 */

#include "Particle.h" 
#include "neopixel/neopixel.h" 
#include "Connection.h"

// TODO 
// Connect to Internet --> if networks are available, then connect to internet 
// https://community.particle.io/t/managing-wifi-and-cloud-with-optional-wifi-support-on-device/51079  

// Miscellaneous 
//      If WiFi setup has incorrect credentials, will indefinitely attempt to connect to WiFi 
//          Test what happens with incorrect credentials if one set of credentials are already stored 
//      is_wifi_available() does not work as intended 

// Production 
//      Configure Special Functions 
//      Correct Server URL 

// Test 

/***************
 * STARTUP CONFIGURATION *
 ***************/ 

STARTUP(setUpInternet());
SYSTEM_MODE(SEMI_AUTOMATIC); 
SYSTEM_THREAD(ENABLED);

/***************
 * PERIPHERALS *
 ***************/ 

int btn1 = D5; 
int btn2 = D6; 
int btn3 = D7; 
int twist = A3; 
int collar = A4; 
int inr_emit = DAC; 
int photo_vcc = A5; 
int photo_out = A0; 
int charger = D0; 
int battery_level = A1; 

Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, D3); 

/***************
 * GLOBALS *
 ***************/ 

String device_id = ""; 

enum event_id { btn1_id, btn2_id, btn3_id, col_id, dispense_id }; 

bool btn1pressed = false; 
bool btn2pressed = false; 
bool btn3pressed = false; 
bool collar_unlocked = false; 

long sleep_time = 0; 
bool dispense_interrupt = false; 
long dispense_interrupt_time = 0; 
int dispense_interrupt_duration = 3000; 

bool charger_connected = false; 

bool is_setup_mode = false; 
int num_setup_btn_presses = 0; 
int CODE_SIZE = 6; 

long check_wifi_time; 
int check_wifi_delay = 3000; 
TCPClient client; 
String server_URL = "aqueous-falls-74814.herokuapp.com"; 
// String server_URL = "medlocksolutions.com"; 
// String server_URL = "6c50e3b367c3.ngrok.io"; 
bool is_data_sent = false; 

/***************
 * FUNCTION DECLARATIONS *
 ***************/ 

// Setup functions  
void set_pins(); 

// Check buttons, dispenses, collar, and charger 
void check_btns(); 
bool check_btns_special_func(); 
void dispense_isr(); 
void check_dispense(); 
void check_collar(); 
void check_charger(); 

// Lights 
void flash_lights(int num, int r, int g, int b); 
void hold_lights(int time, int r, int g, int b); 
void loading_lights(int num, int r, int g, int b); 
void wakeup_lights(bool isSetup); 
void low_battery_lights(); 
void flash_error_code(); 

// Registration setup mode  
void check_btns_setup(); 

// Sleep 
void check_sleep();
void reset_sleep_time(); 
void sleep_sequence(); 

// WiFi 
void begin_listening(); 
void exit_listening_isr(); 
void check_auto_listening(); 
bool is_wifi_available(); 
void check_wifi(); 

// Data Handler 
void store_data(unsigned long value, int type); 
unsigned long encoded(int type, unsigned long value); 
unsigned long timestamp(); 
void print_events(); 
void clear_user_memory(); 
bool is_memory_full(); 

// Data Transmission 
void send_data(); 
void transmit_to_server(String data_str); 
String get_server_res(); 
void handle_server_res(String res); 
String format_json(); 
String decode_name(unsigned long value); 

void setup() {

    set_pins(); 

    strip.begin(); 
    strip.clear(); 
    strip.show();

    Serial.begin(9600); 

    char get_id[24]; EEPROM.get(2, get_id); get_id[25] = 0; 
    long id_check; EEPROM.get(2, id_check); 
    device_id = String(get_id); 
    if(get_id[0] == 0 || id_check == -1 || id_check == 4294967295 || id_check == 0) {
        Serial.println(id_check); 
        Serial.println(get_id); 
        is_setup_mode = true; 
    }

    if(!is_setup_mode) {
        attachInterrupt(twist, dispense_isr, RISING); 
        digitalWrite(inr_emit, HIGH); 
        digitalWrite(photo_vcc, HIGH); 
        if(is_memory_full()) { 
            strip.clear(); 
            for(int i = 0; i < 4; i++) { strip.setPixelColor(i, 24, 13, 24); } 
            strip.show(); 
        } else { wakeup_lights(false); }
    } else {
        EEPROM.clear(); 
        wakeup_lights(true); 
    }

    // check_charger(); 

    // IMPLEMENT: Battery saving wifi logic 
    // WiFi.on(); 
    // check_wifi_time = millis() + check_wifi_delay; 

    // TEST: flashing code over WiFi 
    // System.disableUpdates(); 

    // TEST: only attempt connection if there are credentials 
    Particle.connect(); 
    WiFi.setListenTimeout(300); 

    if(!WiFi.hasCredentials()) { begin_listening(); } 

    reset_sleep_time(); 
    
}

void loop() {

    // IMPLEMENT: Battery saving wifi logic 
    // check_wifi(); 
    check_auto_listening(); 
    check_charger(); 
    
    if(!is_setup_mode) {
        check_dispense(); 

        if(!dispense_interrupt) {
        check_btns(); 
        // DEBUG: uncomment for production 
        check_collar(); 
        }
    } else {
        check_btns_setup(); 
    }

    check_sleep(); 

}

/***************
 * SETUP FUNCTIONS *
 ***************/ 

void set_pins() {
    pinMode(btn1, INPUT_PULLDOWN); 
    pinMode(btn2, INPUT_PULLDOWN); 
    pinMode(btn3, INPUT_PULLDOWN); 
    pinMode(twist, INPUT); 
    pinMode(collar, INPUT_PULLDOWN); 
    pinMode(inr_emit, OUTPUT); 
    pinMode(photo_vcc, OUTPUT); 
    pinMode(photo_out, INPUT); 
    pinMode(charger, INPUT); 
    pinMode(battery_level, INPUT); 

    pinMode(A7, INPUT); 
    
}

/***************
 * PERIPHERAL FUNCTIONS *
 ***************/ 

void check_btns() {

    delay(20); 

    if(check_btns_special_func()) return; 
    
    if(digitalRead(btn1) == HIGH && !btn1pressed) {  reset_sleep_time();  btn1pressed = true; } 
    if(digitalRead(btn2) == HIGH && !btn2pressed) { reset_sleep_time();  btn2pressed = true; } 
    if(digitalRead(btn3) == HIGH && !btn3pressed) { reset_sleep_time(); btn3pressed = true; } 

    if(digitalRead(btn1) == LOW && btn1pressed) { 
        reset_sleep_time(); flash_lights(3, 0, 20, 0); btn1pressed = false; store_data(timestamp(), btn1_id); delay(300); 
    } 
    
    if(digitalRead(btn2) == LOW && btn2pressed) {
        reset_sleep_time(); flash_lights(3, 20, 20, 0); btn2pressed = false; store_data(timestamp(), btn2_id); delay(300); 
    } 
    
    if(digitalRead(btn3) == LOW && btn3pressed) {
        reset_sleep_time(); flash_lights(3, 20, 0, 0); btn3pressed = false; store_data(timestamp(), btn3_id); delay(300); 
    } 
    
}

bool check_btns_special_func() {
    
    int btn_press_time = 25; 

    if(digitalRead(btn1) == HIGH && digitalRead(btn2) == LOW && digitalRead(btn3) == HIGH) {
        int counter = 0; btn1pressed = false; btn2pressed = false; btn3pressed = false; 
        while(digitalRead(btn1) == HIGH && digitalRead(btn2) == LOW && digitalRead(btn3) == HIGH) { 
            counter++; delay(200); 
            if(counter > btn_press_time) break; 
        }
        if(counter > btn_press_time) { begin_listening(); }
        return true; 
    }

    if(digitalRead(btn1) == LOW && digitalRead(btn2) == HIGH && digitalRead(btn3) == HIGH) {
        int counter = 0; btn1pressed = false; btn2pressed = false; btn3pressed = false; 
        while(digitalRead(btn1) == LOW && digitalRead(btn2) == HIGH && digitalRead(btn3) == HIGH) { 
            counter++; delay(200); 
            if(counter > btn_press_time) break; 
        }
        // IMPLEMENT: Change for Production to only System.reset() 
        if(counter > btn_press_time) { flash_lights(1, 20, 20, 20); EEPROM.clear(); System.reset(); }
        return true; 
    }

    if(digitalRead(btn1) == HIGH && digitalRead(btn2) == HIGH && digitalRead(btn3) == LOW) {
        int counter = 0; btn1pressed = false; btn2pressed = false; btn3pressed = false; 
        while(digitalRead(btn1) == HIGH && digitalRead(btn2) == HIGH && digitalRead(btn3) == LOW) { 
            counter++; delay(200); 
            if(counter > btn_press_time) break; 
        }
        if(counter > btn_press_time) { flash_lights(2, 20, 20, 20); sleep_sequence(); }
        return true; 
    }

    if(digitalRead(btn1) == HIGH && digitalRead(btn2) == HIGH && digitalRead(btn3) == HIGH) {
        int counter = 0; btn1pressed = false; btn2pressed = false; btn3pressed = false; 
        while(digitalRead(btn1) == HIGH && digitalRead(btn2) == HIGH && digitalRead(btn3) == HIGH) { 
            counter++; delay(200); 
            if(counter > 35) break; 
        }
        if(counter > 35) { 
            flash_lights(5, 20, 20, 20); 
            WiFi.clearCredentials(); 
        }
        return true; 
    }

    return false; 
}

void check_dispense() {
    if(analogRead(photo_out) < 1800) { 
        reset_sleep_time(); 
        flash_lights(5, 0, 0, 20); 
        store_data(timestamp(), dispense_id); 
        dispense_interrupt = false; 
        attachInterrupt(twist, dispense_isr, RISING); 
        Serial.println(analogRead(photo_out)); 
        if(Particle.connected()) Particle.publish(String(analogRead(photo_out))); 
    } 

    if(dispense_interrupt && millis() > dispense_interrupt_time) {
      dispense_interrupt = false; 
      attachInterrupt(twist, dispense_isr, RISING); 
      strip.clear(); 
      strip.show(); 
    }
} 

void check_collar() {
    if(digitalRead(collar) == HIGH) {
      reset_sleep_time(); 
      flash_lights(1, 20, 0, 0); 
      if(!collar_unlocked) { store_data(timestamp(), col_id); collar_unlocked = true; } 
    } 

    if(digitalRead(collar) == LOW && collar_unlocked) { collar_unlocked = false; } 
    
} 

void check_charger() { 
    if(digitalRead(charger) == HIGH && charger_connected) {
      charger_connected = false; 
    }
    
    if(digitalRead(charger) == LOW && !charger_connected) {
      reset_sleep_time(); 
      flash_lights(3, 26, 14, 0); 
      charger_connected = true; 
    }
}

void dispense_isr() {
  detachInterrupt(twist); 

  for(int i = 0; i < 4; i++) 
    strip.setPixelColor(i, 0, 0, 20); 
  strip.show(); 

  dispense_interrupt = true; 
  dispense_interrupt_time = millis() + dispense_interrupt_duration; 

}

/***************
 * SETUP FUNCTIONS *
 ***************/ 

void check_btns_setup() {

    if(check_btns_special_func()) return; 

    if(digitalRead(btn1) == HIGH && !btn1pressed) {  reset_sleep_time();  btn1pressed = true; } 
    if(digitalRead(btn2) == HIGH && !btn2pressed) { reset_sleep_time();  btn2pressed = true; } 
    if(digitalRead(btn3) == HIGH && !btn3pressed) { reset_sleep_time(); btn3pressed = true; } 

    if(digitalRead(btn1) == LOW && btn1pressed) { 
        short addr; 
        EEPROM.get(0, addr); 
        Serial.println(addr); 

        reset_sleep_time(); flash_lights(3, 0, 20, 0); btn1pressed = false; store_data(timestamp(), btn1_id); num_setup_btn_presses += 1; 
    } 
    
    if(digitalRead(btn2) == LOW && btn2pressed) {
        reset_sleep_time(); flash_lights(3, 20, 20, 0); btn2pressed = false; store_data(timestamp(), btn2_id); num_setup_btn_presses += 1; 
    } 
    
    if(digitalRead(btn3) == LOW && btn3pressed) {
        reset_sleep_time(); flash_lights(3, 20, 0, 0); btn3pressed = false; store_data(timestamp(), btn3_id); num_setup_btn_presses += 1; 
    } 

    if(num_setup_btn_presses >= CODE_SIZE && WiFi.ready()) {
        Serial.println("Sending data from setup mode. "); 
        Serial.println(num_setup_btn_presses); 
        send_data(); 
        System.reset(); 
    }

}

/***************
 * WIFI FUNCTIONS *
 ***************/ 

void check_auto_listening() {
    if(WiFi.listening()) begin_listening(); 
}

void begin_listening() {
    WiFi.on(); 
    attachInterrupt(btn2, exit_listening_isr, RISING); 
    WiFi.listen(); 

    while(WiFi.listening()) { flash_lights(1, 0, 20, 20); }
    while(WiFi.connecting()) { flash_lights(1, 0, 20, 20); flash_lights(1, 0, 20, 0); }

    detachInterrupt(btn2); 

    if(WiFi.ready()) {
        hold_lights(2000, 0, 20, 0); 
    } else { hold_lights(2000, 20, 0, 0); } 


}

void exit_listening_isr() {
    detachInterrupt(btn2); 
    WiFi.listen(false); 
    sleep_sequence(); 
}

// DEBUG: this isn't working, WiFi always turns off even with recognized networks 
bool is_wifi_available() { 
    WiFiAccessPoint my_aps[5];
    int num_ap = 0; 
    num_ap = WiFi.getCredentials(my_aps, 5); 
    // DEBUG: 
    Serial.println(num_ap); 
    WiFiAccessPoint scanned_aps[20];
    int found_aps = WiFi.scan(scanned_aps, 20);

    for(int i = 0; i < num_ap; i++) {
        for(int j = 0; j < found_aps; j++) {
            if(String(my_aps[i].ssid) == String(scanned_aps[j].ssid)) return true; 
        }
    }
    WiFi.off(); 
    return false; 
}

// IMPLEMENT: Scanning for WiFi networks requires a five second delay from WiFi.on(); 
void check_wifi() {
    if(check_wifi_time == 0) return; 

    if(millis() > check_wifi_time) {
        if(is_wifi_available()) {
            Particle.connect(); 
            check_wifi_time = 0; 
        } else {
            WiFi.off(); 
        }
    }
}


/***************
 * DATA HANDLER *
 ***************/ 

/*** EPROM Layout: 
 * 0-1: Current Address
 * 2-25: ID 
 * 26-29: Current Time 
 * 30 + 4i: Value 
 ***/ 
void store_data(unsigned long value, int type) {

    if(is_memory_full()) return; 

    // Encode Data 
    unsigned long encoded_value = encoded(type, value); 

    // Store Data 
    short curr_address; 
    EEPROM.get(0, curr_address); 
    if(curr_address == -1 || curr_address == 65535 || curr_address == 0) {
        EEPROM.put(0, 30ul); 
        curr_address = 30; 
    }

    if(curr_address > EEPROM.length()) { flash_error_code(); return; }

    EEPROM.put(curr_address, encoded_value); 

    // Update Index 
    curr_address += 4; 
    EEPROM.put(0, curr_address); 

    is_data_sent = false; 

} 

bool is_memory_full() {
    short curr_address; EEPROM.get(0, curr_address); 
    if(curr_address > 2000) return true; 
    else return false; 
}

unsigned long encoded(int type, unsigned long value) {
    unsigned long mask = ~0xF; 
    value &= mask; 
    value |= type; 
    return (unsigned long)value; 	
} 

unsigned long timestamp() {
    // Sync Time 
    // TEST: How long it takes to update time if Time.isValid() is true 
    if(!Time.isValid() && Particle.connected()) {
        Particle.syncTime(); waitUntil(Particle.syncTimeDone); 
        if(Time.isValid()) EEPROM.put(26, Time.now()); 
    }

    return Time.now(); 
}

void print_events() {
    short curr_address; 
    EEPROM.get(0, curr_address); 
    if(curr_address == -1 || curr_address == 65535 || curr_address == 0) { Serial.println("No address stored. "); return; } 

    char get_id[24]; 
    EEPROM.get(2, get_id); 

    unsigned long curr_time; 
    EEPROM.get(26, curr_time); 

    Serial.print("Last Updated: "); Serial.println(curr_time); 
    Serial.print("Data: [ "); 
        for(unsigned int i = 30; i < curr_address; i += 4) { 
            u_int32_t value; 
            EEPROM.get(i, value); 
            Serial.print(value); 
            Serial.print(", "); 
        } 
        Serial.println("]. "); 
} 

void clear_user_memory() {
    print_events(); 
    
    char get_id[24]; 
    EEPROM.get(2, get_id); 
    long curr_time = Time.now(); 

    EEPROM.clear(); 
    EEPROM.put(0, 30ul); 
    EEPROM.put(2, get_id); 
    EEPROM.put(26, curr_time); 

    print_events(); 

} 

/***************
 * DATA TRANSMISSION *
 ***************/ 

void send_data() {

    String json_string = format_json(); 
    String server_res;

    loading_lights(0, 0, 20, 20); 

    if(!WiFi.ready()) { return; } 

    loading_lights(1, 0, 20, 20); 

    transmit_to_server(json_string); 

    loading_lights(2, 0, 20, 20); 

    if (client.connect(server_URL, 80)) {         
        Serial.println("connected"); 
        transmit_to_server(json_string); 
        server_res = get_server_res(); 
        client.stop();
    }
    else { 
        Serial.println("connection failed"); 
        hold_lights(2000, 20, 0, 0); 
        if(is_setup_mode) { System.reset(); } else { return; } 
    } 

    loading_lights(3, 0, 20, 20); 

    handle_server_res(server_res); 

} 

void transmit_to_server(String data_str) {

    client.println("POST /api/dispense HTTP/1.1");
    client.println("Host: " + server_URL);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(strlen(data_str));
    client.println();
    client.println(data_str);
    client.println();

} 

String get_server_res() {

    int wait_count = 0; 
    while(!client.available()) {
        Serial.println("Waiting. "); 
        wait_count++; 
        delay(500); 
        if(wait_count > 20) {
            Serial.println("Server Connection Failed. "); 
            return ""; 
        }
    } 

    Serial.print("Reading Bytes Available: "); Serial.println(client.available());

    char server_res[512]; 
    int server_res_counter = 0; 

    while(client.available()) {
        char c = client.read();
        server_res[server_res_counter] = c; 
        server_res_counter++; 
        if(server_res_counter > 511) {
            if(String(server_res).indexOf("502 Bad Gateway ") != -1) {
                Serial.println("Server Connection Failed. 502 Bad Gateway. "); 
                return String(server_res); 
            }
            break; 
        }
    }

    return String(server_res); 

}

void handle_server_res(String res) {

    Serial.println(res); 
    if(res.indexOf("200 OK") == -1) { hold_lights(2000, 20, 0, 0); sleep_sequence(); return; } 

    if(is_setup_mode) {

        if(res.indexOf("Wrong Code.") != -1) { flash_lights(3, 20, 0, 0); System.reset(); return; } 

        int id_idx; 
        if((id_idx = res.indexOf("Dispenser ID: ")) != -1) {
            int start_idx = id_idx + String("Dispenser ID: ").length(); 
            String id = res.substring(start_idx, start_idx + 24); 

            char id_buf[25]; id.toCharArray(id_buf, 25); 
            EEPROM.put(2, id_buf); 

            char get_id[24]; // get_id[25] = 0; 
            EEPROM.get(2, get_id); 
            Serial.print("Get ID: "); 
            Serial.println(get_id); 

        }

        loading_lights(4, 0, 20, 20); 
        clear_user_memory(); 
        hold_lights(2000, 0, 20, 0); 
        System.reset(); 

    } else {

        if(res.indexOf("Dispenser ID: ") == -1) { hold_lights(2000, 20, 0, 0); return; } 
        loading_lights(4, 0, 20, 20); 
        clear_user_memory(); 
        hold_lights(2000, 0, 20, 0); 
        print_events(); 
    }
}

String format_json() {
    short curr_address; EEPROM.get(0, curr_address); 
    char get_id[24]; EEPROM.get(2, get_id); get_id[25] = 0; 
    String id_str = String(1); 
    if(get_id[0] != 0) { id_str = String(get_id); } // id_str = id_str.substring(0, id_str.length()-1); 

    String res = "{ \"id\": \"" + id_str + "\", \"events\": [ "; 
    if(curr_address != -1) {
        for(unsigned int i = 30; i < curr_address; i += 4) { 
            u_int32_t value; 
            EEPROM.get(i, value); 

            res += " { \"name\": "; 
            res += " \"" + decode_name(value) + "\", "; 
            res += " \"value\": "; 
           // res += "" + String(value >> 4) + " }"; 
            res += "" + String(value) + " }"; 

            if(i < curr_address - 4) { res += ","; }
        } 
    }

    unsigned long last_updated; 
    EEPROM.get(26, last_updated); 
    String time_str = String(1); 
    if(last_updated != 0) time_str = String(last_updated); 
    res += " ], \"current_time\": " + time_str + " }"; 

    Serial.println(res); 
    return res; 

}

String decode_name(unsigned long value) {
    long mask = 0xF; 
    unsigned int code = value & mask; 

    switch(code) {
        case btn1_id: 
            return "Button 1"; break; 
        case btn2_id: 
            return "Button 2"; break; 
        case btn3_id: 
            return "Button 3"; break; 
        case col_id: 
            return "Col Off"; break; 
        case dispense_id: 
            return "Dispense"; break; 
        default: 
            return "Default"; break; 
    }
}



/***************
 * LIGHT FUNCTIONS *
 ***************/ 

void flash_lights(int num, int r, int g, int b) {

    if(is_memory_full()) return; 

    strip.clear(); 
    strip.show(); 
    for(int j = 0; j < num; j++) {
        for(int i = 3; i >= 0; i--) {
            strip.setPixelColor(i, r, g, b); 
            strip.show(); 
        } 
        delay(250); 
        strip.clear(); 
        strip.show(); 
        delay(250); 
    }
    strip.clear(); 
    strip.show(); 
}

void hold_lights(int time, int r, int g, int b) {
    strip.clear(); 
    for(int i = 0; i < 4; i++) strip.setPixelColor(i, r, g, b); 
    strip.show(); 
    delay(time); 
    strip.clear(); strip.show(); 
}

void loading_lights(int num, int r, int g, int b) {
    strip.clear(); 
    for(int i = 3; i >= 4-num; i--) { strip.setPixelColor(i, r, g, b); }
    strip.show(); 
}

void wakeup_lights(bool isSetup) {
    int r; int g; int b; 
    if(!isSetup) { r = 0; g = 0; b = 20; } else { r = 25; g = 18; b = 19; } 
    low_battery_lights(); 

    strip.clear(); strip.show(); 
    for(int i = 3; i >= 0; i--) {
        strip.setPixelColor(i, r, g, b); 
        strip.show(); 
        delay(200); 
    } 
    strip.clear(); strip.show(); 
} 

void low_battery_lights() {
    Serial.print("Battery Level: "); Serial.println(analogRead(battery_level)); 
    Particle.publish(String(analogRead(battery_level))); 
    if(analogRead(battery_level) > 500) return; 

    strip.clear(); strip.show(); 
    for(int i = 0; i < 3; i++) {
        strip.setPixelColor(3, 20, 0, 0); strip.setPixelColor(2, 20, 0, 0); strip.show(); 
        delay(250); 
        strip.clear(); strip.setPixelColor(3, 20, 0, 0); strip.show(); 
        delay(250); 
    }
    strip.clear(); strip.show(); 

}

void flash_error_code() {
    Serial.println("Error code. "); 
    strip.clear(); strip.show(); 
    for(int i = 0; i < 3; i++) {
        strip.clear(); strip.setPixelColor(3, 20, 0, 0); strip.setPixelColor(2, 20, 0, 0); strip.show(); 
        delay(250); 
        strip.clear(); strip.setPixelColor(1, 20, 0, 0); strip.setPixelColor(0, 20, 0, 0); strip.show(); 
        delay(250); 
    }
    strip.clear(); strip.show(); 
} 

/***************
 * AUXILIARY FUNCTIONS *
 ***************/ 

void check_sleep() {
    if(millis() >= sleep_time) {
        if(WiFi.ready() && !is_setup_mode && !is_data_sent) { 
            is_data_sent = true; 
            send_data(); 
        }

        if(!(digitalRead(charger) == LOW)) sleep_sequence(); 
        else reset_sleep_time(); 
    }
} 

void reset_sleep_time() {
    sleep_time = millis() + 60000; 
}

void sleep_sequence() {

    for(int i = 4; i > 0; i--) {
        strip.clear(); 
        for(int j = 0; j < i; j++) {
            strip.setPixelColor(j, 0, 0, 20); 
        }
        strip.show(); 
        delay(200); 
    } 
    strip.clear(); strip.show(); 
    Serial.println("Going to sleep. "); 
    System.sleep(SLEEP_MODE_DEEP); 
}
