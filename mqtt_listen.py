import paho.mqtt.client as mqtt
from time import sleep
host_ip = "192.168.43.153" #IP of Wemos 2 or SmartPhone Camera.

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("hello/#")
    print("Listening")

def on_message(client, userdata, message):
    decoded_msg = str(message.payload.decode('utf-8'))
    if decoded_msg == "Activation Signal":
        print("Activation Signal triggered")
    print("Received message " + str(message.payload.decode('utf-8')))

client = mqtt.Client()
client.username_pw_set("Device1","Password123")
client.on_connect = on_connect
client.on_message = on_message
client.connect(host_ip, 1883, 60)
client.loop_forever()