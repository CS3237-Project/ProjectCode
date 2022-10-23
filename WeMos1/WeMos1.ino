#include <ESP8266WiFi.h>
#include "EspMQTTClient.h"

#define imuSwitch D7
#define cameraSwitch D8
#define buzzerPin D3
#define greenLED D5

//const char* ssid = "HUAWEI nova 3i"; // To fill out
//const char* wifiPassword = "23456789"; // To fill out
const char* ssid = "SINGTEL-48EB"; // To fill out
const char* wifiPassword = ""; // To fill out

//const char* mqttBrokerServer = "192.168.43.153"; //The DigitalOcean IP Address as we hosting the MQTT Broker there. For this test, i set it up using my laptop IP.
const char* mqttBrokerServer = "192.168.1.244";
const char* mqttUsername = "Device2";
const char* mqttPassword = "Password123";

bool imuStatus = false;
bool cameraStatus = false;
bool playBadPostureAlert = false;
bool flashLED = false;

volatile unsigned long imu_prev_press_time;
volatile unsigned long imu_current_press_time;

volatile unsigned long camera_prev_press_time;
volatile unsigned long camera_current_press_time;

EspMQTTClient client(
  ssid,
  wifiPassword,
  mqttBrokerServer,  // MQTT Broker server ip
  mqttUsername,   // Can be omitted if not needed
  mqttPassword,   // Can be omitted if not needed
  "Wemos1",     // Client name that uniquely identify your device
  1883          // The MQTT port, default to 1883. this line can be omitted
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
    //  determineWemosStatus();
    }
    imu_prev_press_time = millis();
  }
}

IRAM_ATTR void cameraSwtichTriggered(){
  camera_current_press_time = millis();
  if((camera_current_press_time - camera_prev_press_time) > 250){
    if (cameraStatus == false){
      client.publish("message/Activation","Camera Activation Signal On");
      cameraStatus = true;
    }else{
      client.publish("message/Activation","Camera Activation Signal Off");
      cameraStatus = false;
    //  determineWemosStatus();
    }
    camera_prev_press_time = millis();
  }
}

//Check to see if WeMos should go to sleep
/*
void determineWemosStatus(){
  if (imuStatus == false && cameraStatus == false){
    //Go to light sleep
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    gpio_pin_wakeup_enable(imuSwitch, GPIO_PIN_INTR_LOLEVEL);
    gpio_pin_wakeup_enable(cameraSwitch, GPIO_PIN_INTR_LOLEVEL);
  }
}
*/

void setLED(const String& message){
  flashLED = true;
}

void onPostureReceived(const String& message){
  if (message == "Bad Posture"){
    playBadPostureAlert = true;
  }
}

void onConnectionEstablished()
{
  client.subscribe("message/Posture",onPostureReceived);
  client.subscribe("message/Acknowledgement",setLED);
}

void setup() {
    Serial.begin(115200);
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    pinMode(greenLED,OUTPUT);
    pinMode(buzzerPin,OUTPUT);
    pinMode(imuSwitch,INPUT_PULLUP);
    pinMode(cameraSwitch,INPUT_PULLUP);
    digitalWrite(buzzerPin,LOW);
    digitalWrite(greenLED,LOW);
    attachInterrupt(digitalPinToInterrupt(imuSwitch),imuSwtichTriggered, RISING);
    attachInterrupt(digitalPinToInterrupt(cameraSwitch),cameraSwtichTriggered, RISING);
    imu_prev_press_time = millis();
    imu_current_press_time = millis();
    camera_prev_press_time = millis();
    camera_current_press_time = millis();
    //Determine what caused the wakeup   
}

void loop() {
  
  if (playBadPostureAlert == true){
    digitalWrite(buzzerPin,HIGH);
    delay(2000);
    digitalWrite(buzzerPin,LOW);
    playBadPostureAlert = false;
  }
  
  //Flash green LED to signify an acknowledgement.
  if (flashLED == true){
    digitalWrite(greenLED,HIGH);
    delay(1000);
    digitalWrite(greenLED,LOW);
    delay(1000);
    flashLED = false;
  }
  //Keep MQTT Client Alive.
  client.loop(); 
}
