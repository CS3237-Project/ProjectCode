#include <ESP8266WiFi.h>
#include <IRremote.h>
#include "EspMQTTClient.h"

#define irReceiver D7 

const char* ssid = "HUAWEI nova 3i"; // To fill out
const char* wifiPassword = "23456789"; // To fill out
const char* mqttBrokerServer = "192.168.43.153"; //The DigitalOcean IP Address as we hosting the MQTT Broker there. For this test, i set it up using my laptop IP.
const char* mqttUsername = "Device1";
const char* mqttPassword = "Password123";
bool activationStatus;

EspMQTTClient client(
  ssid,
  wifiPassword,
  mqttBrokerServer,  // MQTT Broker server ip
  mqttUsername,   // Can be omitted if not needed
  mqttPassword,   // Can be omitted if not needed
  "TestClient",     // Client name that uniquely identify your device
  1883          // The MQTT port, default to 1883. this line can be omitted
);

decode_results results;
IRrecv irrecv(irReceiver);

IRAM_ATTR void IRTriggered(){
  if (irrecv.decode(&results)) {
    if (activationStatus == false){
      client.publish("message/Activation", "Activation Signal On");
    }else{
      client.publish("message/Activation", "Activation Signal Off");
    }
  irrecv.resume(); // Receive the next value
  }
}

void onConnectionEstablished()
{
  client.subscribe("message/Activation", [](const String & payload) {
    Serial.println(payload);
  });
}

void setup() {
    Serial.begin(115200);
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    irrecv.enableIRIn(); //Enable the IR receiver
    attachInterrupt(digitalPinToInterrupt(irReceiver),IRTriggered, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Keep MQTT Client Alive.
  client.loop(); 
}
