#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SSID-Wifi";
const char* pass = "Password-Wifi";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* TOPIC_LED = "imclab/led";
const char* TOPIC_SETPOINT = "imclab/motor/setpoint";
const char* TOPIC_SPEED = "imclab/motor/speed";

WiFiClient espClient;
PubSubClient client(espClient);

// LED
const int LED_PIN = 2;

// MOTOR (3 pin - driver H-bridge)
const int MOTOR_IN1 = 27;
const int MOTOR_IN2 = 26;
const int MOTOR_ENA = 12;

const int PWM_FREQ = 5000;
const int PWM_RES = 8;

int motorSetpoint = 0;

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  
  Serial.print("WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String msg = String((char*)payload);
  
  if (String(topic) == TOPIC_LED) {
    int v = msg.toInt();
    digitalWrite(LED_PIN, v ? HIGH : LOW);
    Serial.print("LED ");
    Serial.println(v ? "ON" : "OFF");
  }
  else if (String(topic) == TOPIC_SETPOINT) {
    motorSetpoint = constrain(msg.toInt(), 0, 255);
    
    if (motorSetpoint > 0) {
      // Set direction FORWARD
      digitalWrite(MOTOR_IN1, HIGH);
      digitalWrite(MOTOR_IN2, LOW);
      
      // Set speed
      ledcWrite(MOTOR_ENA, motorSetpoint);
      
      Serial.print("Motor FORWARD speed: ");
      Serial.println(motorSetpoint);
    } else {
      // STOP motor
      digitalWrite(MOTOR_IN1, LOW);
      digitalWrite(MOTOR_IN2, LOW);
      ledcWrite(MOTOR_ENA, 0);
      
      Serial.println("Motor STOP");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("MQTT...");
    String cid = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    
    if (client.connect(cid.c_str())) {
      Serial.println("OK");
      client.subscribe(TOPIC_LED);
      client.subscribe(TOPIC_SETPOINT);
    } else {
      Serial.print("FAIL ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Motor pins
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  
  // PWM untuk ENA
  ledcAttach(MOTOR_ENA, PWM_FREQ, PWM_RES);
  
  // Motor stop awal
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  ledcWrite(MOTOR_ENA, 0);
  
  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();
  
  // Publish speed feedback
  static unsigned long lastPub = 0;
  if (millis() - lastPub > 500) {
    lastPub = millis();
    client.publish(TOPIC_SPEED, String(motorSetpoint).c_str());
  }
}
