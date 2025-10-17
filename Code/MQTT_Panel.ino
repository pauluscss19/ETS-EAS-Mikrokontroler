#include <WiFi.h>
#include <PubSubClient.h>

// ===== KONFIGURASI WiFi =====
const char* ssid = "Heem";            
const char* password = "09090909";     

// ===== KONFIGURASI MQTT =====
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "esp32/motor/control";

// ===== PIN CONFIGURATION =====
const int ledPin = 2;           // LED built-in ESP32
int motor1Pin1 = 27;            // IN1 L298N
int motor1Pin2 = 26;            // IN2 L298N
int enable1Pin = 12;            // ENA L298N

// ===== PWM SETUP =====
const int freq = 5000;
const int resolution = 8;
int motorSpeed = 200;           // Kecepatan tetap

// ===== MQTT Client =====
WiFiClient espClient;
PubSubClient client(espClient);

// ===== FUNGSI KONEKSI WiFi =====
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts % 10 == 0) {
      Serial.println();
      Serial.print("Status: ");
      Serial.println(WiFi.status());
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("✓ WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Blink LED 3x
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(300);
      digitalWrite(ledPin, LOW);
      delay(300);
    }
  } else {
    Serial.println("");
    Serial.println("✗ WiFi connection FAILED!");
    Serial.print("Status code: ");
    Serial.println(WiFi.status());
  }
}

// ===== CALLBACK MQTT =====
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("📩 Pesan diterima: ");
  
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }
  msg.trim();
  Serial.println(msg);
  
  // ===== MOTOR ON =====
  if (msg == "on" || msg == "ON") {
    Serial.println("🟢 Motor ON");
    digitalWrite(motor1Pin1, HIGH);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, motorSpeed);
    digitalWrite(ledPin, HIGH);
  } 
  
  // ===== MOTOR OFF =====
  else if (msg == "off" || msg == "OFF") {
    Serial.println("🔴 Motor OFF");
    digitalWrite(motor1Pin1, LOW);
    digitalWrite(motor1Pin2, LOW);
    ledcWrite(enable1Pin, 0);
    digitalWrite(ledPin, LOW);
  }
}

// ===== RECONNECT MQTT =====
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    
    String clientId = "ESP32Motor-";
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("✓ MQTT connected!");
      client.subscribe(mqtt_topic);
      Serial.print("Subscribed to: ");
      Serial.println(mqtt_topic);
      
      // Blink LED cepat
      for (int i = 0; i < 5; i++) {
        digitalWrite(ledPin, HIGH);
        delay(100);
        digitalWrite(ledPin, LOW);
        delay(100);
      }
    } else {
      Serial.print("✗ Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retry in 5 seconds...");
      delay(5000);
    }
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 Motor ON/OFF via MQTT ===");
  
  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  
  ledcAttach(enable1Pin, freq, resolution);
  
  // Motor OFF di awal
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(enable1Pin, 0);
  
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// ===== LOOP =====
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}