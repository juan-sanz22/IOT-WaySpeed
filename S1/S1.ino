
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"
#include "DHT.h"

#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LDR_PIN 34
#define LED_PIN 2
#define PRESENCA_PIN 27

WiFiClientSecure client;
PubSubClient mqtt(client);


// Função chamada quando chega uma mensagem no MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];  // Converte os bytes recebidos em texto
  }

  // Esse IF é importante → só executa se o comando for para o S1
  if (String(topic) == TOPIC_S1_ILUM) {
    if (msg == "Acender") {
      digitalWrite(LED_PIN, HIGH);   // Liga LED da iluminação
    } 
    else if (msg == "Apagar") {
      digitalWrite(LED_PIN, LOW);    // Desliga LED
    }
  }
}


// Reconecta ao broker caso caia a conexão
void reconnect() {
  while (!mqtt.connected()) {

    // ID do dispositivo usado para conectar no broker
    if (mqtt.connect("S1_DEVICE", BROKER_USER, BROKER_PASS)) {
      
      // S1 precisa escutar o tópico de iluminação
      mqtt.subscribe(TOPIC_S1_ILUM);

    } else {
      delay(2000);
    }
  }
}



void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  pinMode(PRESENCA_PIN, INPUT);

  dht.begin();      // Inicia o sensor DHT11
  client.setInsecure();  // MQTT com SSL sem verificação de certificado

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(200); }

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  reconnect();    // Conecta ao MQTT
}



void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();  // Mantém conexão ativa

  // --- LEITURA DOS SENSORES ---
  int ldr = analogRead(LDR_PIN);            // LDR (0–4095)
  float temp = dht.readTemperature();       // Temperatura
  float umid = dht.readHumidity();          // Umidade
  int presenca = digitalRead(PRESENCA_PIN); // Sensor PIR (0 ou 1)

  // --- PUBLICAÇÃO NO MQTT ---
  // (Esses publishes enviam os dados para o app e para o professor)
  mqtt.publish(TOPIC_S1_ILUM_SENSOR, String(ldr).c_str());
  mqtt.publish(TOPIC_S1_TEMP, String(temp).c_str());
  mqtt.publish(TOPIC_S1_UMID, String(umid).c_str());
  mqtt.publish(TOPIC_S1_PRESENCA, String(presenca).c_str());

  delay(2000);
}