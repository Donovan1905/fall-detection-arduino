#include <Arduino_LSM6DS3.h>

#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>


char ssid[] = "Donovan";
char pass[] = "bleeps1234";

char serverAddress[] = "192.168.163.174"; 
int port = 3000;
bool fallCheck = false;
int spikeCount = 0;
float prevSpikeTime = 1000000;


WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;
float lastLowTime = 1000000;
float savedA;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Started");

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println("Hz");

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                  
    status = WiFi.begin(ssid, pass);
  }

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

}

void loop() {
  float x, y, z;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
  }

  float A = sqrt(pow(x, 2)+pow(y, 2)+pow(z, 2));

  if (abs(A-1) > 4.5 && !fallCheck) {
      lastLowTime = millis();
      savedA = A;
      fallCheck = true;
  }
  Serial.println(spikeCount);
  if (abs(A-1) > 4.5) { 
    prevSpikeTime = millis();
    Serial.println("spikeCount" + String(spikeCount));
    if (fallCheck || spikeCount == 0){
      spikeCount++;
    }
  }
  if (millis() - lastLowTime > 3000 && fallCheck) {
    if(!(spikeCount >= 4)) {
    Serial.println("TOMBERRRRRRRRRRRRRRRR");
        
    String contentType = "application/x-www-form-urlencoded";
    String postData = "name=Denise&fall=" + String(savedA);
    client.post("/api", contentType, postData);

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);
    lastLowTime = 1000000;
    fallCheck=false;
    spikeCount = 0;
    } else {
      lastLowTime = 1000000;
      fallCheck=false;
      spikeCount = 0; 
    }
  }

  delay(100);
}