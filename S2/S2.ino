#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

// -------- Pinos --------
#define TRIG1 5
#define ECHO1 18
#define TRIG2 4
#define ECHO2 19
#define LED_PIN 2

WiFiClientSecure client;
PubSubClient mqtt(client);

bool controleManual = false; // Se receber comando pelo app

// -------- Função para medir distância --------
float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(5);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duracao = pulseIn(echo, HIGH, 20000); // timeout 20ms
  if (duracao == 0) return 999; // sem leitura válida

  return (duracao * 0.0343) / 2;
}

// -------- Callback MQTT --------
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  if (String(topic) == TOPIC_ILUM) {
    controleManual = true;
    if (msg.equalsIgnoreCase("Acender")) {
      digitalWrite(LED_PIN, HIGH);
    } else if (msg.equalsIgnoreCase("Apagar")) {
      digitalWrite(LED_PIN, LOW);
    }
  }
}

// -------- Reconectar MQTT --------
void reconnect() {
  while (!mqtt.connected()) {
    String id = "ESP32_" + String(random(0xffff), HEX);
    if (mqtt.connect(id.c_str(), BROKER_USER, BROKER_PASS)) {
      mqtt.subscribe(TOPIC_ILUM);
    } else {
      delay(2000);
    }
  }
}

// -------- SETUP --------
void setup() {
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  client.setInsecure();
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);
  reconnect();
}

// -------- LOOP --------
void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();

  float d1 = medirDistancia(TRIG1, ECHO1);
  float d2 = medirDistancia(TRIG2, ECHO2);

  int p1 = (d1 < 50) ? 1 : 0;
  int p2 = (d2 < 50) ? 1 : 0;

  mqtt.publish(TOPIC_PRESENCA1, String(p1).c_str());
  mqtt.publish(TOPIC_PRESENCA2, String(p2).c_str());

  if (!controleManual) {
    digitalWrite(LED_PIN, (p1 || p2) ? HIGH : LOW);
  }

  delay(1500);
}
