#include<Arduino.h>
#include<MPU9250.h>
#include "helper/calib.h"
#include "helper/handjson.h"
#include "helper/timer.hpp"
#include "WiFi.h"
#include <WiFiUdp.h>

#define ssid "python"
#define password "12345678"
WiFiUDP Udp;

IPAddress serverIP(192,168,137,1);  // IP address of the UDP server
unsigned int localPort = 8080;

MPU9250 mpu[] = {
  MPU9250(Wire,0x68),
  MPU9250(Wire,0x68),
  MPU9250(Wire,0x68),
  MPU9250(Wire,0x68),
  MPU9250(Wire,0x68),
  MPU9250(Wire,0x68),
};


const int num_mpu = sizeof(mpu)/sizeof(mpu[0]);
unsigned long previous_time[num_mpu], current_time;

float deltat = 0.0f;
#define TCAADDR 0x70
void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}
handjson hand;

String output_data()
{
    String payload;
    hand.serialize(payload);
    Serial.println(payload);
    return payload;
}
void setup() {

  Serial.begin(921600);
  Serial.println("Initiating WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
  WiFi.begin(ssid ,password);
  int count = 0;
  while (WiFi.status()!=WL_CONNECTED)
  {
        delay(500);
 
        Serial.println(WiFi.status());
        count++;
        if (count > 10)
        {
            Serial.println("WiFi connection failed");
            ESP.restart();
            break;
        }
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Connected to WiFi");
  Wire.begin();
  Serial.println(F("#####################"));
  Serial.println(F("Starting Initialisation..."));
  Serial.println(F("#####################"));
  for(int i=0;i<num_mpu;i++){
    tcaselect(i);
    Serial.print(F("TCAADDR: "));
    Serial.println(i);
    if(mpu[i].begin() > 0){
      Serial.println(F("MPU9250 is online..."));
      mpu[i].setAccelRange(MPU9250::ACCEL_RANGE_2G);
      mpu[i].setGyroRange(MPU9250::GYRO_RANGE_250DPS);
      mpu[i].setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_20HZ);
      mpu[i].setSrd(19);
      Serial.println(F("MPU9250 is configured..."));
      Serial.print(F("Calibrating MPU9250 no:"));
      Serial.println(i);
      mpu[i].setAccelCalX(accel_bias_x[i], accel_scale_x[i]);
      mpu[i].setAccelCalY(accel_bias_y[i], accel_scale_y[i]);
      mpu[i].setAccelCalZ(accel_bias_z[i], accel_scale_z[i]);
      mpu[i].setGyroBiasX_rads(gyro_bias_x[i]*M_PI/180);
      mpu[i].setGyroBiasY_rads(gyro_bias_y[i]*M_PI/180);
      mpu[i].setGyroBiasZ_rads(gyro_bias_z[i]*M_PI/180);
      mpu[i].setMagCalX(mag_bias_x[i], mag_scale_x[i]);
      mpu[i].setMagCalY(mag_bias_y[i], mag_scale_y[i]);
      mpu[i].setMagCalZ(mag_bias_z[i], mag_scale_z[i]);
      Serial.println(F("MPU9250 is calibrated..."));
      previous_time[i] = millis();
    }
    else{
      Serial.println(F("MPU9250 is not online..."));
      Serial.println(F("Check your wiring..."));
      while(1) ;
    }
  }
  Serial.println(F("MPU9250 is ready to go..."));
  
  Udp.begin(localPort);
}
void loop() {
  Timer frame;
  for(int i=0;i<num_mpu;i++){
    tcaselect(i);
    mpu[i].readSensor();
    float ax = mpu[i].getAccelX_mss();
    float ay = mpu[i].getAccelY_mss();
    float az = mpu[i].getAccelZ_mss();
    float gx = mpu[i].getGyroX_rads();
    float gy = mpu[i].getGyroY_rads();
    float gz = mpu[i].getGyroZ_rads();
    float mx = mpu[i].getMagX_uT();
    float my = mpu[i].getMagY_uT();
    float mz = mpu[i].getMagZ_uT();
    current_time = millis();
    double deltat = (current_time - previous_time[i])/1000.0;
    previous_time[i] = current_time;
    // mpu[i].ComplementaryFilter(ax, ay, az, gx, gy, gz, deltat);
    mpu[i].MadgwicksQuaternionUpdateIMU(ax, ay, az, gx, gy, gz, deltat);
    Serial.println();
    hand.updatehand(i,mpu[i].getQ0(),mpu[i].getQ1(),mpu[i].getQ2(),mpu[i].getQ3());
  }

    String output=output_data();
    const char* output_str = output.c_str();

    Udp.beginPacket(serverIP, localPort);
    for (int i = 0; i < strlen(output_str); i++) {
    Udp.write(output_str[i]);
    }
    Udp.endPacket();
    delay(400);
  
}