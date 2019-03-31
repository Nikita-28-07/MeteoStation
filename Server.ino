#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RtcDS3231.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> 

RtcDS3231<TwoWire> Rtc(Wire);

const char* ssid = "mi5";
const char* password = "12345678";
String nameFile = "data.csv";
File myFile;
WiFiUDP Udp;
IPAddress IP;

unsigned int localUdpPort = 4210;

char incomingPacket[255];
String out;

void setup() {
  /*Serial.begin(115200);
  Serial.println();
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Print the IP address
  Serial.println(WiFi.localIP());
*/
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.autoConnect("Meteoserver");
  Serial.println("connected...yeey :)");
  
  Udp.begin(localUdpPort); 
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
    
  Serial.print("Initializing SD card...");
  if (!SD.begin(16)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  Serial.println();

  if (!Rtc.IsDateTimeValid()) 
  {
      Serial.println("RTC lost confidence in the DateTime!");
      Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();  
}

void loop() {
 int packetSize = Udp.parsePacket();
      if (packetSize)
      {
        // receive incoming UDP packets
        Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
       
        int len = Udp.read(incomingPacket, 255);
        if (len > 0)
        {
          incomingPacket[len] = 0;
        }
        //Serial.print(incomingPacket);
        Udp.endPacket();  
        if(String(incomingPacket) == "NEWCLIENT"){
           Serial.print(incomingPacket);
         }
        RtcDateTime now = Rtc.GetDateTime();
        char datestring[20];
        printDateTime(now,datestring);
        
        Serial.print(datestring);
        Serial.println();
        
        out = Udp.remoteIP().toString() + "," + datestring + "," + String(incomingPacket);
        myFile = SD.open(nameFile, FILE_WRITE);
        if (!myFile){
          Serial.println("failed open file: " + nameFile);
          while (1);
        }
        else Serial.println("File open: " + nameFile);
        myFile.println(out);
        Serial.println(out); 
        myFile.close();
      }
}
#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt, char *datestring)
{
   sprintf(datestring, "%d/%d/%d %d:%d:%d",     //%d allows to print an integer to the string
          dt.Year(),   //get year method
          dt.Month(),  //get month method
          dt.Day(),    //get day method
          dt.Hour(),   //get hour method
          dt.Minute(), //get minute method
          dt.Second()  //get second method
         );
}
