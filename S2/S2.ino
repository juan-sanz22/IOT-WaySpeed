#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"

WiFiClientSecure client;
PubSubClient mqtt(client);

// Função de callback (chamada quando uma mensagem é recebida)
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Mensagem recebida no tópico: ");
  Serial.println(topic);
  Serial.print("Conteúdo: ");
  Serial.println(msg);

  if (String(topic) == TOPIC_ILUM) {
    if (msg == "Acender") {
      digitalWrite(2, HIGH);
      Serial.println("Luz ligada!");
    } 
    else if (msg == "Apagar") {
      digitalWrite(2, LOW);
      Serial.println("Luz desligada!");
    }
  }
}

void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Tentando conectar ao broker MQTT...");
    String BoardID = "S2_" + String(random(0xffff), HEX);
    
    // Tenta conectar (usuário e senha vazios estão OK)
    if (mqtt.connect(BoardID.c_str(), BROKER_USER, BROKER_PASS)) {
      Serial.println("Conectado!");
      mqtt.subscribe(TOPIC_ILUM);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(mqtt.state());
      Serial.println(" tentando novamente em 2 segundos...");
      delay(2000);
    }
  }
}

void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);

  client.setInsecure();  // SSL sem verificação de certificado
  
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);
  
  reconnect(); // Conecta ao broker
}

void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }

  String mensagem = "Nome: Juan - Salave salve quebrada";
  mqtt.publish(TOPIC_ILUM, mensagem.c_str());

  mqtt.loop();
  delay(2000);
}