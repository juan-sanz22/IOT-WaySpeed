#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <env.h>

WiFiClientSecure client;
PubSubClient mqtt(client);

void setup() {
  Serial.begin(115200);
  client.setInsecure();
  Serial.println("Conectando ao WiFi"); //apresenta a mensagem na tela
  WiFi.begin(WIFI_SSID,WIFI_PASS); //tenta conectar na rede
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); // mostra "....."
    delay(200);
  }
  Serial.println("\nConectado com Sucesso!");

  Serial.println("Conectando ao Broker...");
  mqtt.setServer(BROKER_URL, BROKER_PORT);
  String BoardID = "s2";
  while(!mqtt.connected()){
    BoardID += String(random(0xffff),HEX);
    mqtt.connect(BoardID.c_str() , BROKER_USER, BROKER_PORT;
    Serial.print(".");
    delay(200);
  }
  Serial.println("\nConectado ao Broker!");
  mqtt.subscribe(TOPIC_ILUM);
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
  if(topic == TOPIC_ILUM && msg == "Acender"){
    digitalWrite(2,HIGH);
    if(topic == TOPIC_ILUM && msg == "Apagar"){
  if(topic == TOPIC_ILUM && msg == "Acender"){
    digitalWrite(2,HIGH);
    if(topic == TOPIC_ILUM && msg == "Apagar"){
    digitalWrite(2,LOW);
  }
}