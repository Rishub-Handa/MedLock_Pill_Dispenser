#include "Connection.h"
#include "Particle.h"
#include "softap_http.h"


// IDEA: 
// The timestamp does not need to be accurate to millisecond 
// Encode information in the bottom bits 


// ASSUMPTIONS: 
// No delay in transmission --> Data is set at recorded milliseconds 
// One WebHook for each event 

// TOOD: 
// Collar interrupt 
// Events Array 
// Send Delay Timestamps 
// Interrupts to awake from sleep mode ??? 

String deviceID = "5d14cbb483781a467fed2d72";

// EFFICIENT METHOD OF STORING ALL DATA ??? 

// Option 1: 
int btn1Events[50]; 
int btn2Events[50]; 
int btn3Events[50]; 
int capTurns[100]; 
int dispenses[120]; 

// Option 2: 

// 4 bit encoding 
enum eventsId { btn1, btn2, btn3, capTurn, collarDisengage, dispense }; 

struct Event {
    int id; 
    char name[10]; // CONSTANT ??? 
    // char value[10]; // CONSTANT ??? 
    unsigned long value; 
}; 

int eventsCounter = 0; 
const int eventsLimit = 300; 
// Store in SRAM during Deep Sleep Mode. Explore retained further. 4 KB storage 
// EEPROM is flash memory, only 2 KB 

STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY)); 
retained unsigned long eventsEncoded[eventsLimit]; 

// The first address contains the index of the current address, which is a multiple of 8. 
size_t EEPROM_length = EEPROM.length(); 

int led = A2;
int photoresistorVCC = D0;
int photoresistor = A0;
int capSwitch = A3; // Using Hall Effect Sensors instead of Switch 
int button1 = D5;
int button2 = D4;
int button3 = D3; // Use this to wake up from sleep 
int hallEffectVCC = D2;
int hallEffectReading = A4;
int capRotationHEVCC = D1; 
int capRotationHEReading = A5; 

bool capEngaged = false; 
bool ledOn = false; 
bool collarDisengaged = false; 
bool btn1pressed = false; 
bool btn2pressed = false; 
bool btn3pressed = false; 

unsigned int ledTimeout = 10000; 
unsigned int ledInitialTime = 0; 

// NOT SURE OF THIS VALUE 
int ledThreshold = 2500; 



// void sendData(const char *event, const char *value); 
void sendData(String event, String value); 
// void storeData(const char *event, const char* value); 
void storeData(Event event); 
void activateLED(); 
void deactivateLED(); 
void wakeUpFunction(void); 
unsigned long encoded(unsigned long value, int code); 




// Press SETUP for 3 seconds to make the Photon enter Listening mode
// Navigate to http://192.168.0.1 to setup Wi-Fi 
STARTUP(setUpInternet()); 

void setup() {
    
    Serial.println("Setup"); 
    
    sendData("initial_time", String(millis())); 
    
    pinMode(led, OUTPUT);
  
    pinMode(photoresistorVCC, OUTPUT);
    pinMode(photoresistor, INPUT);
    
    pinMode(capSwitch, INPUT);
    
    pinMode(button1, INPUT);
    pinMode(button2, INPUT);
    pinMode(button3, INPUT);
    
    pinMode(hallEffectVCC, OUTPUT);
    pinMode(hallEffectReading, INPUT);
    
    pinMode(capRotationHEVCC, OUTPUT); 
    pinMode(capRotationHEReading, INPUT); 
    
    attachInterrupt(button1, wakeUpFunction, RISING); 
    
    // REVIEW THIS 
    digitalWrite(hallEffectVCC, HIGH); 
    digitalWrite(capRotationHEVCC, HIGH); 

    Serial.begin(9600);

}


void loop() {
    
    if(digitalRead(button2) == HIGH) {
        Serial.println("Button 1. "); 
    }
    
    if(digitalRead(button1) == HIGH && !btn1pressed){
        btn1pressed = true; 
        // const char *value = (const char *)millis(); 
        // String value = String(millis()); 
        // sendData("button_1", value); 


        Event event = { btn1, "button_1", millis() }; 
        storeData(event); 
        
        Serial.println("Button 1 Pressed"); 
        delay(500); 
    }
    
    if(digitalRead(button1) == LOW && btn1pressed) {
        btn1pressed = false; 
    }
    
    if(digitalRead(button2) == HIGH && !btn2pressed){
        btn2pressed = true; 
        // const char *value = (const char *)millis(); 
        // String value = String(millis()); 
        // sendData("button_2", value); 
        
        Event event = { btn2, "button_2", millis() }; 
        storeData(event); 
        
        Serial.println("Button 2 Pressed"); 
        delay(500); 
        
        // System.sleep(button1, RISING); 
    }
    
    if(digitalRead(button2) == LOW && btn2pressed) {
        btn2pressed = false; 
    }
    
    if(digitalRead(button3) == HIGH && !btn3pressed){
        btn3pressed = true; 
        // const char *value = (const char *)millis(); 
        // String value = String(millis()); 
        // sendData("button_3", value); 
        
        Event event = { btn3, "button_3", millis() }; 
        storeData(event); 
        
        Serial.println("Button 3 Pressed");
        delay(500);
    }
    
    if(digitalRead(button3) == LOW && btn3pressed) {
        btn3pressed = false; 
    }
    
    if(analogRead(hallEffectReading) > 1960 && !collarDisengaged) {
        collarDisengaged = true; 
        // const char *value = (const char *)millis(); 
        // String value = String(millis()); 
        // String value = String(analogRead(hallEffectReading)); 
        // sendData("collar_disengaged", value); 
        
        Event event = { collarDisengage, "col_off", millis() }; 
        storeData(event); 
        
        Serial.println("Collar Disengaged. " + String(analogRead(hallEffectReading))); 
    } 
    
    if(collarDisengaged && analogRead(hallEffectReading) < 1950) {
        collarDisengaged = false; 
        Serial.println("Collar Engaged. " + String(analogRead(hallEffectReading))); 
        // Transmit Collar Reengagement Data Point ??? 
    }
    
    // I HAVE A GUT FEELING THAT THIS SYSTEM IS NOT ROBUST 
    // 1. Send event to the server 
    // 2. Set up LED to begin dispense detection 
    if((analogRead(capRotationHEReading) > 1960) && !capEngaged) {
        capEngaged = true; 
        // const char *value = (const char *)millis(); 
        // TRANSMIT TURNED DATA ??? 
        // String value = String(analogRead(capRotationHEReading)); 
        // sendData("cap_turned", value); 
        
        unsigned long value = analogRead(capRotationHEReading); 
        Event event = { capTurn, "cap_turn", value }; 
        storeData(event); 

        activateLED(); 
        Serial.println("Pill Bottle Turned. "); 
    } else if(capEngaged && (analogRead(capRotationHEReading) < 1950)) {
        capEngaged = false; 
    }
    
    if(ledOn) {
        if(millis() > ledInitialTime + ledTimeout) {
            // sendData("deactivated", String(millis())); 
            deactivateLED(); 
        }
        
        // CALIBRATE THIS CALCULATION 
        if(analogRead(photoresistor) < ledThreshold) {
            deactivateLED(); 
            // const char *value = (const char *)millis(); 
            // String value = String(millis()); 
            // String value = String(analogRead(photoresistor)); 
            // sendData("dispenses", value); 
            
            Event event = { dispense, "dispenses", millis() }; 
            storeData(event); 
            
        }
        

    }

    

}

void sendData(String event, String value) {
    
    // Log Data 
    

    // Send data if available internet. 
    /* 
    if(WiFi.ready()) {
        
        
        
        for(int i = 0; i < eventsCounter; i++) {
            // sprintf(); 
        }
        
        // Array of Events 
    } 
    */ 
    
    // Single Event 
    Particle.publish("RishubDB", "{\"my-id\":\"" + deviceID + "\", \"my-event\":\"" + event + "\", \"my-value\":\"" + value + "\"}", 60, PRIVATE);
}

// void storeData(const char *event, const char *value) {
    
// }

// Store the data compressed and send it out with the name String 

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
            event.id == collarDisengage || 
            event.id == dispense ) && 
        currentAddress < EEPROM_length - 8) {
        EEPROM.put(currentAddress, encoded_value); 
        currentAddress += 8; 
        EEPROM.put(0, currentAddress); 
    }
    
}

void activateLED() {
    digitalWrite(led, HIGH); 
    digitalWrite(photoresistorVCC, HIGH); 
    ledOn = true; 
    ledInitialTime = millis(); 
    delay(500); 
}

void deactivateLED() {
    digitalWrite(led, LOW); 
    digitalWrite(photoresistorVCC, LOW); 
    ledOn = false; 
    ledInitialTime = 0; 
}

void wakeUpFunction(void) {
    digitalWrite(led, HIGH); 
    delay(1000); 
    digitalWrite(led, LOW); 
} 

unsigned long encoded(unsigned long value, int code) { 
    long mask = ~0u << 4; 
    value &= mask;
    value |= code; 
    
    return value; 	
}






