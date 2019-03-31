#include <EEPROM.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)

#define sensorCodeBME280 0
Adafruit_BME280 bme; // I2C

String NameClient = "Датчик 1";
WiFiUDP Udp;
IPAddress IP(192,168,43,113);

unsigned int localUdpPort = 4210;

char incomingPacket[255];
String out;

void writeEEPROM(int startAdr, String writeString) {
  EEPROM.begin(512); //Max bytes of eeprom to use
  yield();
  yield();
  Serial.println();
  //write to eeprom
  int charLength = writeString.length();
  Serial.println("Записываем в EEPROM");
  for (int i = 0; i < charLength; ++i)
    {
      EEPROM.write(startAdr + i, writeString[i]);
      Serial.print("Wrote: ");
      Serial.println(writeString[i]);
    }
}

IPAddress readEEPROM(int startAdr, int maxLength, char* dest) {
  Serial.println("Читаем EEPROM");
  EEPROM.begin(512);
  for (int i = 0; i < maxLength; ++i)
  {
    dest += char(EEPROM.read(startAdr + i));
    if(dest[i]='\0') break;//break when end of sting is reached before maxLength
  }
  String data(dest); 
  int ind1 = data.indexOf('.');  
  int str1 = data.substring(0, ind1).toInt();   
  int ind2 = data.indexOf('.', ind1+1);
  int str2 = data.substring(ind1+1, ind2).toInt() ;  
  int ind3 = data.indexOf('.', ind2+1);
  int str3 = data.substring(ind2+1, ind3).toInt();
  int str4 = data.substring(ind3+1, data.length()).toInt();
  IPAddress ret(str1,str2,str3,str4);
  Serial.print("Прочитали: ");
  Serial.println(dest);
  return ret;
}

void searchServer(){
  Serial.print("Ищим сервер");
  if(checkServer()) return;
  while(1){
    IPAddress broadcastIp(255,255,255,255);
    Udp.beginPacket(broadcastIp,localUdpPort);
    out = "NEWCLIENT" + NameClient;
    char charBuf[50];
    out.toCharArray(charBuf, 255);
    Udp.write(charBuf);
    Serial.print(".");
    Udp.endPacket();
    delay(1000);
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      IP = Udp.remoteIP();
      writeEEPROM(4,Udp.remoteIP().toString());
      Serial.println("Сервер найден!!!");
      Serial.println("Его IP-адрес: " + Udp.remoteIP().toString());
    }
  }
}
bool checkServer(){
    Serial.println("Проверяем сохраненные данные");
    char *str;
    readEEPROM(4,15,str);
    IPAddress IPserver(String(str));
    Udp.beginPacket(IPserver,localUdpPort);
    out = "NEWCLIENT" + NameClient;
    char charBuf[50];
    out.toCharArray(charBuf, 255);
    Udp.write(charBuf);
    Udp.endPacket();
    delay(1000);
    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      Serial.println("Данные актуальны");
      Serial.println("Сервер найден!!!");
      Serial.println("Его IP-адрес: " + Udp.remoteIP().toString());
      return 1;
    }
    Serial.println("Данные неактуальны");
    return 0;
}
void setup ()  {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect("MeteoClient");
  Serial.println("Подключились :)");
  Udp.begin(localUdpPort); 
  searchServer();
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  Serial.println(F("Проверка BME280..."));
  if (!bme.begin()) {
      Serial.println("Датчик BME280 не найден, проверь соединение!");
      while (1);
  }
  Serial.println(F("BME280 найден"));
}
  
void loop () { 
      Serial.println(IP.toString() + "," + localUdpPort);
      Udp.beginPacket(IP, localUdpPort); 
      out = String(sensorCodeBME280) + "," + String(bme.readTemperature()) + "," + String(bme.readPressure() / 100.0F) + "," + String(bme.readHumidity());
      char charBuf[50];
      out.toCharArray(charBuf, 255);
      Udp.write(charBuf);
      Udp.endPacket();
      Serial.println(charBuf);
      delay(1000);
}
