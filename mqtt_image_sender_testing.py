import paho.mqtt.client as mqtt
import json
import time
import os
import base64

host_ip = "165.22.110.229"  # Broker IP
#host_ip ="192.168.43.153"
index = 0
all_images = []

def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("message/posture")
    print("Listening")

##Connect VCC pointing towards my wallet.
def on_message(client, userdata, message):
    print(str(message.payload))
    sendImage(all_images[index])

def sendImage(filename):
    if os.path.isfile(filename):
        with open(filename, "rb") as image:
            encoded_string = base64.b64encode(image.read())
        client.publish("image",encoded_string)
        global index
        print("Image sent:"+all_images[index])
        index = index + 1

def get_filename(directory):
    from os import listdir
    from os.path import isfile, join
    global all_images
    all_images = [join(directory, file) for file in listdir(directory) if isfile(join(directory, file))]

client = mqtt.Client()
client.username_pw_set("Device1", "Password123")
client.on_connect = on_connect
client.on_message = on_message
client.connect(host_ip, 1883, 60)
get_filename("Images")
sendImage(all_images[0])
client.loop_forever()
