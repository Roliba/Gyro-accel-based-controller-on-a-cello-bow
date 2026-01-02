#include <ESP8266WiFi.h>

// Nome della rete WiFi e password
const char *ssid = "ESP8266_Network";
const char *password = "12345678";  // Almeno 8 caratteri

void setup() {
    Serial.begin(115200);
    Serial.println();
    
    // Configura ESP8266 come Access Point
    WiFi.softAP(ssid, password);
    
    Serial.print("Rete WiFi creata. IP: ");
    Serial.println(WiFi.softAPIP());
}

void loop() {
    // Il loop può rimanere vuoto oppure si possono gestire le connessioni
}
