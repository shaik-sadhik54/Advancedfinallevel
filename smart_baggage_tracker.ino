#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

const char* server = "garbage-monitoring-syste-f1f56.appspot.com";

static const int RXPin = 5, TXPin = 4;   // GPIO 4=D2(connect Tx of GPS) and GPIO 5=D1(connect Rx of GPS)
static const uint32_t GPSBaud = 9600; //if Baud rate 9600 didn't work in your case then use 4800

TinyGPS gps; // The TinyGPS object

WiFiClient client;

SoftwareSerial ss(RXPin, TXPin);  // The serial connection to the GPS device

float lastLat = 0.0; // Variable to store the last known latitude
float lastLon = 0.0; // Variable to store the last known longitude

char auth[] = "G-QNFYPNepq8AcNDfUf6QtgUCstqSm3";              //Your Project authentication key
char ssid[] = "IOT_J";                                       // Name of your network (HotSpot or Router name)
char pass[] = "10101010";                                      // Corresponding Password

const char *ssid1 =  "Param";    
const char *pass1 =  "87654321";

unsigned long lastCheckTime = 0; // Variable to store the last time coordinates were checked
unsigned long checkInterval = 60000; // Check coordinates every 60 seconds

void setup()
{
  Serial.begin(9600);
  Serial.println();
  ss.begin(GPSBaud);
  
  Serial.println("Connecting to ");
  Serial.println(ssid1);

  WiFi.begin(ssid1, pass1);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop()
{
  delay(5000);

  while (ss.available() > 0) 
  {
    char c = ss.read();
    if (gps.encode(c))
      displayInfo();
  }
}

void displayInfo()
{ 
  float latitude, longitude;
  unsigned long fix_age;
  gps.f_get_position(&latitude, &longitude, &fix_age);

  Serial.print("LAT:  ");
  Serial.println(latitude, 6);
  Serial.print("LONG: ");
  Serial.println(longitude, 6);

  // Check if enough time has passed since the last check
  if (millis() - lastCheckTime > checkInterval) {
    // If the coordinates have significantly changed, send them to the server
    if (abs(latitude - lastLat) > 0.0001 || abs(longitude - lastLon) > 0.0001) {
      sendCoordinates(latitude, longitude);
      lastLat = latitude;
      lastLon = longitude;
    }
    lastCheckTime = millis();
  }
}

void sendCoordinates(float latitude, float longitude) {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connect(server, 80)) {
      String postStr = "lat=";
      postStr += String(latitude, 6);
      postStr += "&long=";
      postStr += String(longitude, 6);
      postStr += "\r\n\r\n";

      client.print("POST /location HTTP/1.1\n");
      client.print("Host: garbage-monitoring-syste-f1f56.appspot.com\n");
      client.print("Connection: close\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(postStr.length());
      client.print("\n\n");
      client.print(postStr);
    } else {
      Serial.println("Connection Failed");
    }
    client.stop();
  }
}
