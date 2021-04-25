#Fichier readserial.py
import time
import serial
import json

ser = serial.Serial(
 port='COM3',
 baudrate = 9600,
 parity=serial.PARITY_NONE,
 stopbits=serial.STOPBITS_ONE,
 bytesize=serial.EIGHTBITS,
 timeout=10
)
counter=0

while 1:                                    # read the data recieved on the serial port
 # On lit la ligne
 x = ser.readline()
 x = x.rstrip()
 x = x.decode("utf-8")                      # decode the data from byte to unicode 8bit format
 print ("Valeur : {}".format(x))

 in_payload = json.loads(x)                 # decode the JSON data 
 
 out_payload = {"command" : 0}              # create a dict that will recieve the output data
 
 if in_payload['light'] < 100 :             # using the recieved data determine which command to give
     if (in_payload['temperature'] >28):
         out_payload["command"] = 1
     elif (in_payload['temperature'] < 20):
         out_payload["command"] = 2
     else :
         out_payload["command"]= 12
 else :
     if (in_payload['temperature'] >25):
         out_payload["command"] = 1
     elif (in_payload['temperature'] < 18):
         out_payload["command"] = 2
     else :
         out_payload["command"]= 12     

 out_payload = json.dumps(out_payload)      # using the dict, create a string of data in json format
 ser.write(out_payload.encode('utf-8'))     # send out the json data after encoding it into bytes type
 
 #ser.flush()
 