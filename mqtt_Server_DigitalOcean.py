import paho.mqtt.client as mqtt
import json

host_ip = "192.168.43.153" #Digital Ocean Server IP [Broker]

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("message/IMUData")
    print("Listening")

def on_message(client, userdata, message):
    topic_name = str(message.topic)
    if (topic_name == "message/IMUData"):
        recv_dict = json.loads(message.payload)
        degX = recv_dict["X Angle"]
        degY = recv_dict["Y Angle"]
        degZ = recv_dict["Z Angle"]
    
    print("X Degree: " + str(degX))
    print ("Y Degree: " + str(degY))
    print ("Z Degree: " + str(degZ))

client = mqtt.Client()
client.username_pw_set("Device1","Password123")
client.on_connect = on_connect
client.on_message = on_message
client.connect(host_ip, 1883, 60)
client.loop_forever()