TCPClient client; 

byte server[] = { 74, 125, 224, 72 }; // Google
int led = DAC; 
bool disconnected = false; 

void setup() {
    
    WiFi.connect(); 
    
    pinMode(led, OUTPUT); 
    
    Serial.begin(9600);
    waitFor(Serial.isConnected, 30000);
    
    Serial.println("connecting...");
    
    if (client.connect("http://ae7f5c58.ngrok.io", 80)) { 
        
        const char* jsonDataString = "{\"A\":\"57051\",\"B\":\"30808\",\"C\":\"37.98\",\"D\":\"1.029\",\"E\":\"8.54\",\"F\":\"300.6\",\"G\":\"8.915\",\"H\":\"42.31\"}";
        
        Serial.println("connected");
        
        // Learn how to send a correct POST request. 
        
        client.println("POST http://ae7f5c58.ngrok.io/api/dispense HTTP/1.1");
        client.println("Host: http://ae7f5c58.ngrok.io");
        client.println("Content-Type: application/json");
        // client.println("Content-Length: 0");
        // client.println();
        
        client.print("content-length: ");
        client.println(strlen(jsonDataString));
        client.println();
        client.println(jsonDataString);
        client.println();
        client.stop();
    }
    else
    {
        Serial.println("connection failed");
    }
    
    // connectToGoogle(); 

}

void loop() {
    
    if(!disconnected) { 
    
        if (client.available())
        {
            char c = client.read();
            Serial.print(c);
        }
    
        if (!client.connected())
        {
            Serial.println();
            Serial.println("disconnecting.");
            client.stop();
            disconnected = true; 
            // for(;;);
        }
    
    }


}

void connectToGoogle() {
    
    if (client.connect("www.google.com", 80))
    {
        Serial.println("connected");
        client.println("GET /search?q=unicorn HTTP/1.0");
        client.println("Host: www.google.com");
        client.println("Content-Length: 0");
        client.println();
    }
    else
    {
        Serial.println("connection failed");
    }
} 



