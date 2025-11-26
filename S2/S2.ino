#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

// === SENSOR 1 ===
#define TRIG1 5
#define ECHO1 18

// === SENSOR 2 ===
#define TRIG2 4
#define ECHO2 19

// LED
#define LED_PIN 2

WiFiClientSecure client;
PubSubClient mqtt(client);

bool controleManual = false; // TRUE quando o app enviar "Acender" ou "Apagar"

// Função para medir distância
float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(3000);
  digitalWrite(trig, LOW);

  long duracao = pulseIn(echo, HIGH);
  float distancia = (duracao * 0.0343) / 2;

  return distancia;
}

// ======= CALLBACK (recebe mensagens do broker) =======
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("\nMensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  if (String(topic) == TOPIC_ILUM) {

    controleManual = true;

    if (msg.equalsIgnoreCase("Acender")) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("💡 Luz ligada pelo app!");
    } 
    else if (msg.equalsIgnoreCase("Apagar")) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("💤 Luz apagada pelo app!");
    }
  }
}

// ======= RECONNECT =======
void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Tentando conectar ao broker MQTT... ");
    String clientId = "S2_" + String(random(0xffff), HEX);

    if (mqtt.connect(clientId.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");
      mqtt.subscribe(TOPIC_ILUM);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 3 segundos...");
      delay(3000);
    }
  }
}

// ======= SETUP =======
void setup() {
  Serial.begin(115200);

  pinMode(TRIG1, OUTPUT);
  pinMode(ECHO1, INPUT);
  pinMode(TRIG2, OUTPUT);
  pinMode(ECHO2, INPUT);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setInsecure();
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  reconnect();
}

// ======= LOOP PRINCIPAL =======
void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();

  // === SENSOR 1 ===
  float dist1 = medirDistancia(TRIG1, ECHO1);
  Serial.print("Sensor 1 distância: ");
  Serial.print(dist1);
  Serial.println(" cm");

  int presenca1 = (dist1 < 50 && dist1 > 0) ? 1 : 0;
  mqtt.publish(TOPIC_PRESENCA1, String(presenca1).c_str());
  Serial.print("Presença 1: ");
  Serial.println(presenca1);

  // === SENSOR 2 ===
  float dist2 = medirDistancia(TRIG2, ECHO2);
  Serial.print("Sensor 2 distância: ");
  Serial.print(dist2);
  Serial.println(" cm");

  int presenca2 = (dist2 < 50 && dist2 > 0) ? 1 : 0;
  mqtt.publish(TOPIC_PRESENCA2, String(presenca2).c_str());
  Serial.print("Presença 2: ");
  Serial.println(presenca2);

  // === MODO AUTOMÁTICO DO LED ===
  if (!controleManual) {
    if (presenca1 == 1 || presenca2 == 1) {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("⚠ Movimento detectado → LED ON");
    } else {
      digitalWrite(LED_PIN, LOW);
      Serial.println("Sem movimento → LED OFF");
    }
  }

  delay(3000);
}
