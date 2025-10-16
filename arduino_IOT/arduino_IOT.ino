#include <WiFi.h>

const String SSID = "nome";
const String Pass = "senha";

void setup() {
  Serial.begin(115200);
  Serial.prntln("Conectado ao Wifi");
  WiFi.begin(SSID,PASS);
  while(WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.prntkn("\nConectando com sucesso");

}

void loop() {
  // put your main code here, to run repeatedly:

}