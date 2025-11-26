//Bibliotecas
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"
#include "DHT.h"

//Definições dos pinos
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LDR_PIN 34
#define LED_PIN 2
#define PRESENCA_PIN 27

WiFiClientSecure client;
PubSubClient mqtt(client);


// função callback, que é chamada quando o ESP recebe um comando pelo MQTT.
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];  // Converte os bytes recebidos em texto e Ela verifica se a mensagem chegou no tópico de iluminação.
  }

  
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
  Serial.begin(115200); // Inicia o serial

  pinMode(LED_PIN, OUTPUT); 
  pinMode(PRESENCA_PIN, INPUT); // Configura os pinos

  dht.begin();      
  client.setInsecure(); // inicia o DHT

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(200); } // Conecta ao WiFi

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback); // Define o callback 

  reconnect();    // Conecta ao MQTT
}



void loop() {
  if (!mqtt.connected()) reconnect();
  mqtt.loop();  // Se desconectar, chama reconnect().

  //  lê os sensores
  int ldr = analogRead(LDR_PIN);            // LDR (0–4095)
  float temp = dht.readTemperature();       // Temperatura
  float umid = dht.readHumidity();          // Umidade
  int presenca = digitalRead(PRESENCA_PIN); // Sensor de presença 

  // Sensores conectado nos tópicos certos 
  mqtt.publish(TOPIC_S1_ILUM_SENSOR, String(ldr).c_str());
  mqtt.publish(TOPIC_S1_TEMP, String(temp).c_str());
  mqtt.publish(TOPIC_S1_UMID, String(umid).c_str());
  mqtt.publish(TOPIC_S1_PRESENCA, String(presenca).c_str());

  delay(2000);  // Envia a cada 2 segundos
}