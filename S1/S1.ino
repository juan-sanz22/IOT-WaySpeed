// Bibliotecas 
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "env.h"
#include "DHT.h"

// Pino do DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pinos do sensor ultrassônico
#define TRIG_PIN 22
#define ECHO_PIN 23

// LED normal
#define LED_PIN 19

// Sensor LDR
#define LDR_PIN 34

// Pinos do LED RGB
#define R_PIN 14
#define G_PIN 26
#define B_PIN 25

WiFiClientSecure client;
PubSubClient mqtt(client);


// Função callback é chamada sempre que chega comando pelo MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  // Converte a mensagem recebida para texto
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  // Se a mensagem for do tópico de iluminação do S1
  if (String(topic) == TOPIC_S1_ILUM) {

    // Liga o LED e acende o RGB em vermelho
    if (msg == "Acender") {
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(R_PIN, 255);
      digitalWrite(G_PIN, 0);
      digitalWrite(B_PIN, 0);
    }

    // Desliga tudo
    else if (msg == "Apagar") {
      digitalWrite(LED_PIN, LOW);
      digitalWrite(R_PIN, 0);
      digitalWrite(G_PIN, 0);
      digitalWrite(B_PIN, 0);
    }
  }
}


// Função que reconecta ao servidor MQTT quando a conexão cai
void reconnect() {
  while (!mqtt.connected()) {

    // Tenta conectar
    if (mqtt.connect("S1_DEVICE", BROKER_USER, BROKER_PASS)) {

      // Quando conecta, volta a escutar o tópico de iluminação
      mqtt.subscribe(TOPIC_S1_ILUM);

    } else {
      delay(2000);  // Aguarda e tenta de novo
    }
  }
}


void setup() {
  Serial.begin(115200);                // Inicia serial para depuração

  pinMode(LED_PIN, OUTPUT);            // LED normal
  pinMode(TRIG_PIN, OUTPUT);           // Trigger do ultrassônico
  pinMode(ECHO_PIN, INPUT);            // Echo do ultrassônico

  pinMode(R_PIN, OUTPUT);              // LED RGB Vermelho
  pinMode(G_PIN, OUTPUT);              // LED RGB Verde
  pinMode(B_PIN, OUTPUT);              // LED RGB Azul

  dht.begin();                          // Inicializa o DHT11
  client.setInsecure();                 // Permite MQTT com SSL sem certificado

  // Conecta ao WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(200); }

  // Configura o servidor MQTT e define o callback
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  mqtt.setCallback(callback);

  reconnect();                          // Conecta ao MQTT
}


// Função para medir distância com o ultrassônico
float medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);         // Pulso para ativar o sensor
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duracao = pulseIn(ECHO_PIN, HIGH);  // Mede o tempo do eco

  float distancia = duracao * 0.034 / 2;   // Fórmula padrão em CM

  return distancia;
}


void loop() {
  if (!mqtt.connected()) reconnect();   // Se cair, reconecta
  mqtt.loop();                          // Mantém a conexão ativa

  // Lê todos os sensores
  float temp = dht.readTemperature();   // Temperatura
  float umid = dht.readHumidity();      // Umidade
  int ldr = analogRead(LDR_PIN);        // Luminosidade
  float distancia = medirDistancia();   // Distância do ultrassônico

  // Publica cada valor no tópico correspondente
  mqtt.publish(TOPIC_S1_TEMP, String(temp).c_str());
  mqtt.publish(TOPIC_S1_UMID, String(umid).c_str());
  mqtt.publish(TOPIC_S1_LDR, String(ldr).c_str());
  mqtt.publish(TOPIC_S1_DISTANCE, String(distancia).c_str());

  delay(2000);  // Envia a cada 2 segundos
}
