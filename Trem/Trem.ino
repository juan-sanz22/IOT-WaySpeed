#include <WiFi.h>
#include <PubSubClient.h>
#include <env.h>

WiFiClient client;
PubSubClient mqtt(client);



void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFi"); //apresenta a mensagem na tela
  WiFi.begin(WIFI_SSID,WIFI_PASS); //tenta conectar na rede
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); // mostra "....."
    delay(200);
  }
  Serial.println("\nConectado com Sucesso!");

  Serial.println("Conectando ao Broker...");
  mqtt.setServer(BROKER_URL,BrokerPort);
  String BoardID = "s2";
  BoardID += String(random(0xffff),HEX);
  mqtt.connect(BoardID.c_str() , Broker_User, Broker_Pass);
  while(!mqtt.connected()){
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe(TOPIC_ILUM)
  Serial.println("\nConectado ao Broker!");
}

void loop() {
String mensagem = "Nome:Juan";
mensagem+= "oi";
mqtt.publish(TOPIC_ILUM , mensagem.c_str());
mqtt.loop();
delay(1000);
}


void callback(char* topic, byte* payload, usigned int length){
  string msg = "";
  for(int i = 0; i < length; i++){
    msg += (char) payload[i]
  }
  if(topic == "TOPIC_ILUM" && msg == "Acender"){
    digitalWrite(2,HIGH);
    if(topic == "TOPIC_ILUM" && msg == "Apagar"){
    digitalWrite(2,LOW);
  }
}