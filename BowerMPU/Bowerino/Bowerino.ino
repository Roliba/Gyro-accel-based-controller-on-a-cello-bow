#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h> // 請在程式庫管理員搜尋 OSC 庫安裝

// WiFi Config
const char* ssid = "ESP8266_Network";
const char* password = "12345678";
const char* destIP = "192.168.4.8";  // IP del computer con Max/MSP
const int destPort = 8000;             // Porta UDP per Max/MSP

// MPU6050 & UDP
Adafruit_MPU6050 mpu;
WiFiUDP udp;

float yaw = 0; // Inizializza yaw
unsigned long lastTime = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin(4, 5);
    // Connessione WiFi
    Serial.print("Connessione a ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnesso al WiFi!");

    // Inizializzazione MPU6050
    Serial.println("Inizializzazione MPU6050...");
    if (!mpu.begin()) {
        Serial.println("MPU6050 non trovato!");
        while (1);
    }
    Serial.println("MPU6050 trovato!");
    lastTime = millis();
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    float ax = a.acceleration.x;
    float ay = a.acceleration.y;
    float az = a.acceleration.z;
    float gz = g.gyro.z;

    // 🔹 Calcolo di Pitch e Roll
    float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    float roll = atan2(ay, az) * 180.0 / PI;

    // 🔹 Calcolo dello Yaw usando il giroscopio
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - lastTime) / 1000.0;  // Secondi
    lastTime = currentTime;
    yaw += gz * deltaTime;  // Integrazione

    sendOSC(pitch, roll, yaw);
    delay(50);
}

// 🔹 Funzione per inviare Pitch, Roll e Yaw via OSC
void sendOSC(float pitch, float roll, float yaw) {
    char buffer[64];  
    int index = 0;

    // Scrive il messaggio "/mpu/orientation"
    index += writeOSCString(buffer + index, "/mpu/orientation");
    index += writeOSCString(buffer + index, ",fff");  // 3 float

    // Aggiunge i valori di Pitch, Roll e Yaw
    index += writeOSCFloat(buffer + index, pitch);
    index += writeOSCFloat(buffer + index, roll);
    index += writeOSCFloat(buffer + index, yaw);

    // Invia il pacchetto UDP
    udp.beginPacket(destIP, destPort);
    udp.write((uint8_t*)buffer, index);
    udp.endPacket();

    Serial.println("OSC inviato!");
}


// void sendOSC(float pitch, float roll, float yaw) {
//     OSCMessage msg("/mpu/orientation");
//     msg.add(pitch);
//     msg.add(roll);
//     msg.add(yaw);

//     udp.beginPacket(destIP, destPort);
//     msg.send(udp); // 自動處理所有 Padding 和 Big-endian 轉換
//     udp.endPacket();
//     msg.empty(); // 釋放記憶體
//     Serial.println("OSC inviato!");
// }

// 🔹 Funzione per scrivere stringhe OSC con padding a 4 byte
int writeOSCString(char* buffer, const char* str) {
    int len = strlen(str);
    strcpy(buffer, str);
    buffer[len] = '\0';  // Terminazione stringa

    int paddedLen = (len + 4) & ~3;  // Allinea a multipli di 4 byte
    memset(buffer + len + 1, 0, paddedLen - len - 1);
    return paddedLen;
}

// 🔹 Funzione per scrivere un float in binario OSC (big-endian)
int writeOSCFloat(char* buffer, float value) {
    uint32_t binValue;
    memcpy(&binValue, &value, sizeof(value));

    buffer[0] = (binValue >> 24) & 0xFF;
    buffer[1] = (binValue >> 16) & 0xFF;
    buffer[2] = (binValue >> 8) & 0xFF;
    buffer[3] = binValue & 0xFF;
    return 4;
}
