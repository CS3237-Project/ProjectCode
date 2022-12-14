#include <MPU9250_WE.h>
#include <Wire.h>
#define MPU9250_ADDR 0x68
#include <ESP8266WiFi.h>
#include "EspMQTTClient.h"
#include <ArduinoJson.h>

const char* ssid = "SINGTEL-48EB"; // To fill out
const char* wifiPassword = "eiseeseido"; // To fill out

const char* mqttBrokerServer = "34.81.217.13";//The DigitalOcean IP Address as we hosting the MQTT Broker there. For this test, i set it up using my laptop IP.
const char* mqttUsername = "Device2";
const char* mqttPassword = "Password123";
//unsigned long clocktime;

/* There are several ways to create your MPU9250 object:
 * MPU9250_WE myMPU9250 = MPU9250_WE()              -> uses Wire / I2C Address = 0x68
 * MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR)  -> uses Wire / MPU9250_ADDR
 * MPU9250_WE myMPU9250 = MPU9250_WE(&wire2)        -> uses the TwoWire object wire2 / MPU9250_ADDR
 * MPU9250_WE myMPU9250 = MPU9250_WE(&wire2, MPU9250_ADDR) -> all together
 * Successfully tested with two I2C busses on an ESP32
 */
MPU9250_WE myMPU9250 = MPU9250_WE(MPU9250_ADDR);
float sequence_of_reading[80];
int num_of_reading = 0;

bool activation_status;
const int capacity = JSON_OBJECT_SIZE(80);

StaticJsonDocument<capacity> IMUData;

EspMQTTClient client(
  ssid,
  wifiPassword,
  mqttBrokerServer,  // MQTT Broker server ip
  mqttUsername,   // Can be omitted if not needed
  mqttPassword,   // Can be omitted if not needed
  "WeMosIMUBottom",     // Client name that uniquely identify your device
  1884          // The MQTT port, default to 1883. this line can be omitted
);

void sendIMUReading(){
//  clocktime=millis();
//  Serial.print("Start Sending: ");
//  Serial.print(clocktime);
//  Serial.println(" ");
  StaticJsonDocument<capacity> IMUData;
  copyArray(sequence_of_reading, sizeof(sequence_of_reading), IMUData);
  String message;
  serializeJson(IMUData, message);
  client.publish("message/IMUData",message);
//  Serial.print("Done Sending: ");
//  clocktime=millis();
//  Serial.print(clocktime);
//  Serial.println(" ");
}

void calibrateIMU(){
  /* The slope of the curve of acceleration vs measured values fits quite well to the theoretical 
   * values, e.g. 16384 units/g in the +/- 2g range. But the starting point, if you position the 
   * MPU9250 flat, is not necessarily 0g/0g/1g for x/y/z. The autoOffset function measures offset 
   * values. It assumes your MPU9250 is positioned flat with its x,y-plane. The more you deviate 
   * from this, the less accurate will be your results.
   * The function also measures the offset of the gyroscope data. The gyroscope offset does not   
   * depend on the positioning.
   * This function needs to be called at the beginning since it can overwrite your settings!
   */
  
  /*  This is a more accurate method for calibration. You have to determine the minimum and maximum 
   *  raw acceleration values of the axes determined in the range +/- 2 g. 
   *  You call the function as follows: setAccOffsets(xMin,xMax,yMin,yMax,zMin,zMax);
   *  Use either autoOffset or setAccOffsets, not both.
   */
  Serial.println("Position you MPU9250 flat and don't move it - calibrating...");
  delay(3000);
  myMPU9250.autoOffsets();
  Serial.println("Done!");

  /*  Sample rate divider divides the output rate of the gyroscope and accelerometer.
   *  Sample rate = Internal sample rate / (1 + divider) 
   *  It can only be applied if the corresponding DLPF is enabled and 0<DLPF<7!
   *  Divider is a number 0...255
   */
  myMPU9250.setSampleRateDivider(49);
  /*  MPU9250_ACC_RANGE_2G      2 g   
   *  MPU9250_ACC_RANGE_4G      4 g
   *  MPU9250_ACC_RANGE_8G      8 g   
   *  MPU9250_ACC_RANGE_16G    16 g
   */
  myMPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
  /*  Enable/disable the digital low pass filter for the accelerometer 
   *  If disabled the bandwidth is 1.13 kHz, delay is 0.75 ms, output rate is 4 kHz
   */
   
  myMPU9250.enableAccDLPF(true);
 /*  Digital low pass filter (DLPF) for the accelerometer (if DLPF enabled) 
   *  MPU9250_DPLF_0, MPU9250_DPLF_2, ...... MPU9250_DPLF_7 
   *   DLPF     Bandwidth [Hz]      Delay [ms]    Output rate [kHz]
   *     0           460               1.94           1
   *     1           184               5.80           1
   *     2            92               7.80           1
   *     3            41              11.80           1
   *     4            20              19.80           1
   *     5            10              35.70           1
   *     6             5              66.96           1
   *     7           460               1.94           1
   */
  myMPU9250.setAccDLPF(MPU9250_DLPF_6);
}

void onActivationStatusReceived(const String& message){
  if (message == "IMU Activation Signal On"){
    calibrateIMU();
    activation_status = true;
    num_of_reading = 0;
  }else if(message == "IMU Activation Signal Off"){
    activation_status = false;
  }
  client.publish("message/Acknowledgement","Flash LED");
}

void getIMUReading(){
  //clocktime=millis();
  //Serial.print("Start Reading: ");
  //Serial.print(clocktime);
  xyzFloat corrAngleRaw = myMPU9250.getAngles();
  sequence_of_reading[num_of_reading] = corrAngleRaw.x;
  sequence_of_reading[num_of_reading + 1] = corrAngleRaw.y;
  num_of_reading = num_of_reading + 2;
  //clocktime=millis();
  //Serial.print("End Reading: ");
  //Serial.print(clocktime);
}


void onConnectionEstablished()
{
    client.subscribe("message/Activation",onActivationStatusReceived);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED_BUILTIN,OUTPUT);
  activation_status = false;
  digitalWrite(LED_BUILTIN,LOW);
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  client.setMaxPacketSize(3000);
  digitalWrite(LED_BUILTIN,HIGH);
}

void loop() {
  if (activation_status == true){
    getIMUReading();
    if(num_of_reading == 80){  
      sendIMUReading();
      num_of_reading = 0;
    }
    delay(50); //50 milliseconds delay to prevent overwhelming the Mosquitto Server.
  }
  client.loop();
}
