#include <AWS_IOT.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

AWS_IOT hornbill;
#define SEALEVELPRESSURE_HPA (1012.00)
Adafruit_BMP280 bmp; // I2C

char WIFI_SSID[]="hackathon2019";
char WIFI_PASSWORD[]="fearlesscoder";
char HOST_ADDRESS[]="a1lgiu6au69lhu-ats.iot.us-west-2.amazonaws.com";
char CLIENT_ID[]= "client id";
char TOPIC_NAME[]= "$aws/things/toaster/shadow/update";
int toasterPin = 2;


int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

unsigned long delayTime;

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}

void setup() {
    pinMode(toasterPin, OUTPUT);
    Serial.begin(115200);
    delay(2000);

    while (status != WL_CONNECTED)
    {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(WIFI_SSID);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        // wait 5 seconds for connection:
        delay(5000);
    }

    Serial.println("Connected to wifi");

    if(hornbill.connect(HOST_ADDRESS,CLIENT_ID)== 0)
    {
        Serial.println("Connected to AWS");
        delay(1000);

        if(0==hornbill.subscribe(TOPIC_NAME,mySubCallBackHandler))
        {
            Serial.println("Subscribe Successfull");
        }
        else
        {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    }
    else
    {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }

    delay(2000);

    Serial.println(F("BMP280 test"));

    bool status;
    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
    status = bmp.begin(0x76);  //I2C address can be 0x77 or 0x76 (by default 0x77 set in library)
    if (!status) {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        while (1);
    }
    
    Serial.println("-- Default Test --");
    delayTime = 1000;

    Serial.println();
}

void loop() { 
    delay(delayTime);
    
    if(msgReceived == 1)
    {
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
    }
//    if(tick >= 5)   // publish to topic every 5seconds
//    {
//        tick=0;
//        sprintf(payload,"{ \"state\": { \"desired\": {\"reading\": {\"temperature": %f, \"pressure": %f}}}}", returnTemperature(), returnPressure());
//        if(hornbill.publish(TOPIC_NAME,payload) == 0)
//        {        
//            Serial.print("Publish Message:");
//            Serial.println(payload);
//        }
//        else
//        {
//            Serial.println("Publish failed");
//        }
//    }  
//    vTaskDelay(1000 / portTICK_RATE_MS); 
//    tick++;

}


float returnValues() {
    Serial.print("Approx. Altitude = ");
    Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");
    Serial.println();
}

float returnTemperature() {
   float temp = bmp.readTemperature()
//   Serial.print("Temperature = ");
//   Serial.print(temp);
//   Serial.println(" *C");
   return temp;
}

float returnPressure() {
    float pressure = bmp.readPressure() / 100.0F;
//    Serial.print("Pressure = ");
//    Serial.print(pressure);
//    Serial.println(" hPa");
    return pressure;
}

