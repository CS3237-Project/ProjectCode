import base64
from datetime import datetime
from pathlib import Path

import paho.mqtt.client as mqtt

IMAGE_FOLDER_PATH = './images/'


host_ip = '128.199.104.154'  # Broker IP


def on_connect(client, userdata, flags, rc):
    print('Connected with result code' + str(rc))
    client.subscribe('image')
    print('Listening')


def on_message(client, userdata, message):
    Path(IMAGE_FOLDER_PATH).mkdir(parents=True, exist_ok=True)
    imgdata = base64.b64decode(message.payload)
    today = IMAGE_FOLDER_PATH + datetime.now().strftime('%d-%m-%y_%H-%M-%S')
    filename = today + '.jpg'
    with open(filename, 'wb') as f:
        f.write(imgdata)
    print('Saved to ' + filename)


def setup(hostname):
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(hostname)
    client.loop_start()
    return client


def main():
    setup(host_ip)

    while True:
        pass


if __name__ == '__main__':
    main()
