#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>
#include "env.h"

#define SERVO_PIN 19
#define PIR_PIN 21  // Escolha o pino digital que conecta o sensor PIR

WiFiClientSecure wifiClient;
PubSubClient mqtt(wifiClient);
Servo servoMotor;

// -------------------- VARIÁVEL DE ESTADO --------------------
int servoState = 0; // 0 = sem presença, 1 = presença
int pirState = 0;   // Estado atual do sensor PIR

// -------------------- CALLBACK MQTT --------------------
void mqttCallback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (unsigned int i = 0; i < length; i++) msg += (char)message[i];

  if (msg == "1" && servoState == 0) { // Presença detectada e servo ainda em 0°
    servoMotor.write(90);
    servoState = 1;
  } 
  else if (msg == "0" && servoState == 1) { // Sem presença e servo ainda em 90°
    servoMotor.write(0);
    servoState = 0;
  }
}

// -------------------- RECONNECT MQTT --------------------
void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.println("Conectando ao MQTT...");
    String clientId = "ESP32_Servo_" + String(random(0xffff), HEX);
    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      mqtt.subscribe(TOPIC_PRESENCA1);
      Serial.println("Conectado e inscrito no tópico PRESENCA1!");
    } else {
      Serial.print("Falha, rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(115200);

  // Servo
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0); // Começa em 0°
  servoState = 0;

  // PIR
  pinMode(PIR_PIN, INPUT);

  // WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.println("\nWiFi conectado!");

  // MQTT
  wifiClient.setInsecure();  // Desativa verificação SSL (só para testes)
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(mqttCallback);
  reconnectMQTT();
}

// -------------------- LOOP --------------------
void loop() {
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  // --- Leitura do PIR ---
  int leitura = digitalRead(PIR_PIN); // 0 = sem presença, 1 = presença
  if (leitura != pirState) {          // Só publica se houver mudança
    pirState = leitura;

    // Publica no MQTT
    mqtt.publish(TOPIC_PRESENCA1, String(pirState).c_str());

    // Controla o servo localmente
    if (pirState == 1 && servoState == 0) {
      servoMotor.write(90);
      servoState = 1;
    } else if (pirState == 0 && servoState == 1) {
      servoMotor.write(0);
      servoState = 0;
    }
  }

  delay(100); // Pequena pausa para estabilidade do PIR
}
