#include <ESP8266WiFi.h>
#include <IRremote.h>
#include "EspMQTTClient.h"

#define irReceiver D7
#define greenLED D5 

const char* ssid = "HUAWEI nova 3i"; // To fill out
const char* wifiPassword = "23456789"; // To fill out
const char* mqttBrokerServer = "192.168.43.153"; //The DigitalOcean IP Address as we hosting the MQTT Broker there. For this test, i set it up using my laptop IP.
const char* mqttUsername = "Device1";
const char* mqttPassword = "Password123";
bool activationStatus;
unsigned long prev_press_time;
unsigned long current_press_time;

EspMQTTClient client(
  ssid,
  wifiPassword,
  mqttBrokerServer,  // MQTT Broker server ip
  mqttUsername,   // Can be omitted if not needed
  mqttPassword,   // Can be omitted if not needed
  "TestClient2",     // Client name that uniquely identify your device
  1883          // The MQTT port, default to 1883. this line can be omitted
);

decode_results results;
IRrecv irrecv(irReceiver);

IRAM_ATTR void IRTriggered(){
  /*
  current_press_time = millis();
  Serial.println(irrecv.decode(&results),HEX);
  if (irrecv.decode(&results) && (current_press_time - prev_press_time) > 250) {
    if (activationStatus == false){
      client.publish("message/Activation", "Activation Signal On");
      Serial.println("Activation Signal On");
      activationStatus = true;
    }else{
      client.publish("message/Activation", "Activation Signal Off");
      Serial.println("Activation Signal Off");
      activationStatus = false;
    }
  irrecv.resume(); // Receive the next value
  prev_press_time = millis();
  }
  */
}

void setLED(const String& message){
  if (message == "Turn LED On"){
      digitalWrite(greenLED,HIGH);
  }else{
      digitalWrite(greenLED,LOW);
  }
}

void onConnectionEstablished()
{
  client.subscribe("message/ActivationStatus", setLED);
}

void setup() {
    Serial.begin(115200);
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    irrecv.enableIRIn(); //Enable the IR receiver
    attachInterrupt(digitalPinToInterrupt(irReceiver),IRTriggered, RISING);
    pinMode(greenLED,OUTPUT);
    digitalWrite(greenLED,LOW);
    prev_press_time = millis();
    current_press_time = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  //Keep MQTT Client Alive.
  client.loop(); 
}
