
// Tested Pins 
int inr_emit = DAC;
int photo_vcc = D0;
int photo_out = A0; 

int btn1 = D5;
int btn2 = D6;
int btn3 = D7;

// Untested Pins 
int led_bg_red = D4;
int led_bg_green = D3;
int led_bg_blue = A5;

int cap_switch = A3;

int cap_out = A1; 
int cap_vcc = D1;

int collar_vcc = D2;
int collar_out = A4; 

int x = 10; 

void flash(int led); 
void test_btn_RGB(); 
void test_btn(); 
void set_pins(); 
void test_photo_sensor(); 
void EEPROM_test(); 
void SRAM_test(); 

void setup() { 
    set_pins(); 

    digitalWrite(inr_emit, HIGH); 
    digitalWrite(photo_vcc, HIGH); 

}

void loop() {
    test_photo_sensor(); 

    delay(50); 
} 


void EEPROM_test() {
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

void SRAM_test() { 
    // Will Require Reset 
    Serial.println(x);
    x = 20;
    Serial.println(x);
    delay(10000); 
    System.sleep(SLEEP_MODE_DEEP, 10); 
}


// Test Successful 10/17/19 
void test_photo_sensor() { 

    if(analogRead(photo_out) < 20) {
        Serial.println("Dispense. "); 
    }

}

// Successful 10/17/19 
void test_btn() {
    if(digitalRead(btn1) == HIGH) {
        Serial.println("Button 1. "); 
    }
    if(digitalRead(btn2) == HIGH) {
        Serial.println("Button 2. "); 
    } 
    if(digitalRead(btn3) == HIGH) {
        Serial.println("Button 3. "); 
    }
} 

void flash(int led) {
    for(int i = 0; i < 3; i++) {
        digitalWrite(led, HIGH); 
        delay(1000); 
        digitalWrite(led, LOW); 
        delay(1000); 
    }
}

void set_pins() {
    pinMode(inr_emit, OUTPUT); 
    pinMode(photo_vcc, OUTPUT); 
    pinMode(photo_out, INPUT); 

    pinMode(btn1, INPUT); 
    pinMode(btn2, INPUT);  
    pinMode(btn3, INPUT); 
}