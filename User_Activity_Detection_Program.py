import paho.mqtt.client as mqtt
import numpy as np
import json
from keras.models import load_model
import tensorflow as tf
# To preserve tensorflow state in multithreaded env.
from tensorflow.python.keras.backend import set_session
from datetime import datetime
import pytz

#Specify timezone
tz = pytz.timezone('Asia/Singapore')

MODEL_FILE = "Activity.hd5"

#Create a tensorflow session. For state presevation in Multi-thread env.
session = tf.compat.v1.Session(graph= tf.compat.v1.Graph())

dict = {0:'Sitting',1:'Standing',2:'Walking'}

sitting_standing_too_long = False

host_ip = "34.81.217.13"  # Broker IP
num_of_consecutive_walking = 0
num_of_consecutive_sitting = 0
num_of_consecutive_standing = 0
user_activity = "Sitting"
prev_user_activity = "Sitting"
corrupted_packet = False
trans_combined_data = []
camera_status = "OFF"
IMU_status = "OFF"

def get_date_time():
    dateTime = datetime.now(tz).strftime('%d-%m-%y_%H-%M-%S')
    inputDate = dateTime[6:8] + '-' + dateTime[3:5] + '-' + dateTime[0:2]
    inputTime = dateTime[9:11] + ':' + dateTime[12:14] + ':' +dateTime[15:17]
    return inputDate + " " +inputTime

prev_time_walked = datetime.now(tz)
first_time_walking = datetime.now(tz)


def on_connect(client, userdata, flags, rc):
    print("Connected with result code: " + str(rc))
    client.subscribe("message/IMUData")
    client.subscribe("message/Activation")
    print("Listening")

##Connect VCC pointing towards my wallet.
def on_message(client, userdata, message):
    global camera_status
    global IMU_status
    global dict
    global prev_time_walked
    global first_time_walking
    global sitting_standing_too_long
    global trans_combined_data
    global num_of_consecutive_walking
    global num_of_consecutive_sitting
    global num_of_consecutive_standing
    global user_activity
    global prev_user_activity
    global corrupted_packet
    global tz
    topic_name = str(message.topic)

    if(topic_name == "message/Activation" and message.payload.decode("utf-8") == "IMU Activation Signal On"):
        sitting_standing_too_long = False
        user_activity = "Sitting"
        IMU_status = "ON"
        camera_status = "OFF"
        first_time_walking = datetime.now(tz)
        prev_time_walked = datetime.now(tz)

    if(topic_name == "message/Activation" and message.payload.decode("utf-8") == "IMU Activation Signal Off"):
        IMU_status = "OFF"
        camera_status = "OFF"

    if (topic_name == "message/IMUData"):
#        f = open("ReceivedIMUData.txt", "a")
#        f.write(str(message.payload)+"\n")
#        t = open("TimeCheck.txt", "a")
        try:
            combined_data = json.loads(message.payload)
            corrupted_packet = False
        except:
            corrupted_packet = True
#            f.write("Corrupted Packet \n")
#            t.write("Corrupted Packet \n")
        if (corrupted_packet == False):
            trans_combined_data = combined_data
            trans_combined_data = np.reshape(trans_combined_data,(1,2,40))
            d = open("UserActivity.txt", "a")
            set_session(session)
            result = model.predict(trans_combined_data)
            themax = np.argmax(result)
            print("Result:"+str(result))
            print("Activity Detected:"+dict[themax])
#            t.write("Activity Detected:"+dict[themax] +"," + str(get_date_time())+"\n")

            # Due to fluctuations, we only take it as walking if the user is walking continuously for 6 seconds.
            if(dict[themax] == "Walking"):
                num_of_consecutive_walking = num_of_consecutive_walking + 1
            else:
                num_of_consecutive_walking = 0

            if(dict[themax] == "Sitting"):
                num_of_consecutive_sitting = num_of_consecutive_sitting + 1
            else:
                num_of_consecutive_sitting = 0

            if(dict[themax] == "Standing"):
                num_of_consecutive_standing = num_of_consecutive_standing + 1
            else:
                num_of_consecutive_standing = 0

            ## Detect start of walking
            if (num_of_consecutive_walking >= 2 and user_activity != "Walking"):
                first_time_walking = datetime.now(tz)

            # The user activity state only changes if the same user activity detected two consecutive times.
            if (num_of_consecutive_walking >= 2):
                prev_user_activity = user_activity
                user_activity = "Walking"

            elif(num_of_consecutive_sitting >= 2):
                prev_user_activity = user_activity
                user_activity = "Sitting"

            elif(num_of_consecutive_standing >= 2):
                prev_user_activity = user_activity
                user_activity = "Standing"

            if (prev_user_activity == "Walking" and user_activity != "Walking"):
                prev_time_walked =  datetime.now(tz)

           # Publish Camera Activated if User Activity is standing, else Publish Activity
            if(user_activity == "Sitting" and camera_status == "OFF"):
               client.publish("message/Activation","Camera Activation Signal On")
               camera_status = "ON"
               print("ON")
            elif (user_activity != "Sitting" and camera_status == "ON"):
               client.publish("message/Activation","Camera Activation Signal Off")
               camera_status = "OFF"
               print("OFF")

            print("User Activity: "+user_activity)
#            t.write(" User Activity:" + user_activity)

            d.write(str(get_date_time())+","+user_activity+"\n")

            #Only switch off the buzzer after we have walked for 3 minutes.
            if((datetime.now(tz) - first_time_walking).total_seconds()> 176 and user_activity == "Walking" and IMU_status == "ON"):
                prev_time_walked = datetime.now(tz)
                if (sitting_standing_too_long == True):
                    sitting_standing_too_long = False
                    client.publish("message/Activity","Active")

            #Only switch on the buzzer after we have stopped walking for 1 hour.
            if(((datetime.now(tz) - prev_time_walked).total_seconds()> 3596) and sitting_standing_too_long == False and IMU_status == "ON"):
                client.publish("message/Activity","Inactive")
                sitting_standing_too_long = True
            d.close()
#        f.close()
#        t.close()

client = mqtt.Client()
client.username_pw_set("Device3", "Password123")
client.on_connect = on_connect
client.on_message = on_message
client.connect(host_ip, 1884, 60)

model = load_model(MODEL_FILE)
client.loop_forever()