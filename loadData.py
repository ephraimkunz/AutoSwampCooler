from serial import Serial
import re
import time
import boto3
import decimal

dynamodb = boto3.resource('dynamodb', region_name ='us-east-1', endpoint_url='https://dynamodb.us-east-1.amazonaws.com')
def open_serial_port():
   s = Serial('/dev/ttyACM0', 9600)
   line = s.readline()
   return s

def read_data(s):
    line = s.readline()
    if len(line.split()) > 0:
        timestamp = decimal.Decimal(time.time())
        table = dynamodb.Table('SwampData')
        table.put_item(Item={'timestamp': timestamp, 'data': line})
    time.sleep(2);    

s = open_serial_port()
while 1:
    read_data(s)
