#include <WiFi.h>
#include <PubSubClient.h>

WiFiClient client;
PubSubClient mqtt(client);

const String BrokerURL = "test.mosquitto.org"; //endereço do broker público
const int BrokerPort = 1883;                   //porta do broker público
const String BrokerUser = "";  //usuário do servidor
const String BrokerPass = "";  //senha do servidor

const String SSID = "FIESC_IOT_EDU";  //Nome do WiFi
const String PASS = "8120gv08";  //Senha do WiFi

void setup() {
  Serial.begin(115200);
  Serial.println("Conectando ao WiFi"); //apresenta a mensagem na tela
  WiFi.begin(SSID,PASS); //tenta conectar na rede
  while(WiFi.status() != WL_CONNECTED){
    Serial.print("."); // mostra "....."
    delay(200);
  }
  Serial.println("\nConectado com Sucesso!");

  Serial.println("Conectando ao Broker...");
  mqtt.setServer(BrokerURL.c_str(),BrokerPort);
  String BoardID = "s2";
  BoardID += String(random(0xffff),HEX);
  mqtt.connect(BoardID.c_str() , BrokerUser.c_str() , BrokerPass.c_str());
  while(!mqtt.connected()){
    Serial.print(".");
    delay(200);
  }
  mqtt.subscribe("Topico-DSM14")
  Serial.println("\nConectado ao Broker!");
}

void loop() {
String mensagem = "Nome:Juan";
mensagem+= "oi";
mqtt.publish("Topico-DSM14" , mensagem.c_str());
mqtt.loop();
delay(1000);
}


void callback(char* topic, byte* payload, usigned int length){
  string msg = "";
  for(int i = 0; i < length; i++){
    msg += (char) payload[i]
  }
  if(topic == "iluminacao" && msg == "Acender"){
    digitalWrite(2,HIGH);
    if(topic == "iluminacao" && msg == "Apagar"){
    digitalWrite(2,LOW);
  }
}