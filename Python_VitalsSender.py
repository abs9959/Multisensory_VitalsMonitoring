# -*- coding: utf-8 -*-
"""
Created on Wed May 15 07:00:47 2024

@author: abbyk
"""
import csv
import socket
import time
from threading import Thread

# Define the ESP32 IP address and port
ESP32_IP = '10.121.242.166'  # Replace with your ESP32 IP address
#ESP32_IP = '192.168.0.17'

ESP32_PORT_HR = 12345
ESP32_PORT_BP = 12346

def read_csv(file_path, is_hr=False):
    data = []
    
    with open(file_path, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip the header row
        for row in reader:
            try:
                if is_hr:
                    data.append((int(row[0]), int(row[1]), int(row[2])))
                else:
                    data.append((int(row[0]), int(row[1])))
            except ValueError:
                print(f"Invalid row: {row}")
                
    return data

def send_heart_rate_data(data):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((ESP32_IP, ESP32_PORT_HR))
        for heart_rate_data in data:
            message = f"HR,{heart_rate_data[0]:03d},{heart_rate_data[1]:03d},{heart_rate_data[2]}\n"
            s.sendall(message.encode())
            print("SENT HR")
            time.sleep(heart_rate_data[2])  # Sleep for the duration specified in the row

def send_blood_pressure_data(data):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((ESP32_IP, ESP32_PORT_BP))
        for blood_pressure_data in data:
            message = f"BP,{blood_pressure_data[0]:03d},{blood_pressure_data[1]}\n"
            s.sendall(message.encode())
            print("SENT BP")
            print(message)
            time.sleep(blood_pressure_data[1])  # Sleep for the duration specified in the value2

if __name__ == "__main__":
    heart_rate_file_path = 'Block4.csv'  # Replace with your heart rate CSV file path
    blood_pressure_file_path = 'BP.csv'  # Replace with your blood pressure CSV file path
    
    heart_rate_data = read_csv(heart_rate_file_path, is_hr=True)
    blood_pressure_data = read_csv(blood_pressure_file_path)
    
    # Create and start threads for sending heart rate and blood pressure data
    heart_rate_thread = Thread(target=send_heart_rate_data, args=(heart_rate_data,))
    blood_pressure_thread = Thread(target=send_blood_pressure_data, args=(blood_pressure_data,))
    
    heart_rate_thread.start()
    blood_pressure_thread.start()
    
    heart_rate_thread.join()
    blood_pressure_thread.join()
