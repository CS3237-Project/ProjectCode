#include <ESP8266WiFi.h>
#include "EspMQTTClient.h"

#define imuSwitch D2
#define buzzerPin D3
#define LED D5

const char* ssid = "SINGTEL-48EB"; // To fill out
const char* wifiPassword = "eiseeseido"; // To fill out

const char* mqttBrokerServer = "34.81.217.13"; //The DigitalOcean IP Address as we hosting the MQTT Broker there. For this test, i set it up using my laptop IP.
const char* mqttUsername = "Device2";
const char* mqttPassword = "Password123";

bool imuStatus = false;
bool playInactiveAlert = false;
bool flashLED = false;

volatile unsigned long imu_prev_press_time;
volatile unsigned long imu_current_press_time;

EspMQTTClient client(
  ssid,
  wifiPassword,
  mqttBrokerServer,  // MQTT Broker server ip
  mqttUsername,   // Can be omitted if not needed
  mqttPassword,   // Can be omitted if not needed
  "Wemos1",     // Client name that uniquely identify your device
  1884          // The MQTT port, default to 1883. this line can be omitted
);


IRAM_ATTR void imuSwtichTriggered(){
  imu_current_press_time = millis();
  if((imu_current_press_time - imu_prev_press_time) > 250){
    if (imuStatus == false){
      client.publish("message/Activation","IMU Activation Signal On");
      imuStatus = true;
    }else{
      client.publish("message/Activation","IMU Activation Signal Off");
      imuStatus = false;
    }
    imu_prev_press_time = millis();
  }
}

void setLED(const String& message){
  flashLED = true;
}

void onActivityReceived(const String& message){
  if (message == "Inactive"){
    playInactiveAlert = true;
  }else if(message == "Active"){
    playInactiveAlert = false;
  }
}

void onConnectionEstablished()
{
  client.subscribe("message/Activity",onActivityReceived);
  client.subscribe("message/Acknowledgement",setLED);
}

void setup() {
    Serial.begin(115200);
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    pinMode(LED,OUTPUT);
    pinMode(buzzerPin,OUTPUT);
    pinMode(imuSwitch,INPUT_PULLUP);
    digitalWrite(buzzerPin,LOW);
    digitalWrite(LED,LOW);
    attachInterrupt(digitalPinToInterrupt(imuSwitch),imuSwtichTriggered, RISING);
    imu_prev_press_time = millis();
    imu_current_press_time = millis();
}

void loop() {
  if (playInactiveAlert == true){
    digitalWrite(buzzerPin,HIGH);
  }else{
    digitalWrite(buzzerPin,LOW);
  }
  
  //Flash green LED to signify an acknowledgement.
  if (flashLED == true){
    digitalWrite(LED,HIGH);
    delay(1000);
    digitalWrite(LED,LOW);
    delay(1000);
    flashLED = false;
  }
  //Keep MQTT Client Alive.
  client.loop(); 
}
