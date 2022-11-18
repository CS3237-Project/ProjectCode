import numpy as np
import tensorflow as tf
from tensorflow import keras
import pandas as pd
from scipy import stats
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import OneHotEncoder
from tensorflow.keras.callbacks import ModelCheckpoint
from tensorflow.keras.models import Model, load_model
from tensorflow.keras.optimizers import Adam
import os.path
MODEL_FILE = "Activity.hd5"
STEP = 30
TIME_STEPS = 40

def create_dataset(X, y, time_steps=1, step=1):
    Xs, ys = [], []
    for i in range(0, len(X) - time_steps, step):
        v = X.iloc[i:(i + time_steps)].values
        labels = y.iloc[i: i + time_steps]
        Xs.append(v)
        ys.append(stats.mode(labels)[0][0])
    return np.array(Xs), np.array(ys).reshape(-1, 1)


def create_model():
    model = keras.Sequential()
    model.add(keras.layers.LSTM(units=128,input_shape=(2,TIME_STEPS)))
    model.add(keras.layers.Dropout(rate=0.1))
    model.add(keras.layers.Dense(y_train.shape[1], activation='softmax'))
    adam = Adam(lr = 0.00001)
    model.compile(loss='categorical_crossentropy', optimizer=adam, metrics=['acc'])
    model.summary()
    return model


## Read in the Raw.txt
column_names = ['activity','acc_x_axis', 'acc_y_axis']
df = pd.read_csv('LabelledData.txt', header=None, names=column_names)
df.dropna(axis=0, how='any', inplace=True) #Remove Missing Value
df.head()

# Take a sequence of 2 seconds worth of activity.
df_x, df_y = create_dataset(df[['acc_x_axis','acc_y_axis']], df.activity, TIME_STEPS, STEP)

#Save 20% of data as Test
X_train, X_test,y_train,y_test = train_test_split(df_x, df_y, test_size=0.2,shuffle=True)

X_train = np.reshape(X_train, (X_train.shape[0],X_train.shape[2], X_train.shape[1]))
X_test = np.reshape(X_test, (X_test.shape[0], X_test.shape[2], X_test.shape[1]))

print(y_train)
enc = OneHotEncoder(handle_unknown='ignore', sparse=False)

enc = enc.fit(y_train)

y_train = enc.transform(y_train)
y_test = enc.transform(y_test)

if os.path.exists(MODEL_FILE):
    print("\n*** Model Found. Loading***\n\n")
    model = load_model(MODEL_FILE)
else:
    print("\n*** Creating new model ***\n\n")
    model = create_model()


checkpoint = ModelCheckpoint(MODEL_FILE)

#Train Model
model.fit(X_train,y_train, epochs=40, batch_size = 1, callbacks=[checkpoint],validation_split = 0.1)
model.summary()

print("Testing Data Accuracy:")
model.evaluate(X_test, y_test)