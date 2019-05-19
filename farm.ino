// upload this file to Lambda Management Console under Function Code

#include <AWS_IOT.h>
#include <WiFi.h>

#include <Wire.h>
#include <VL53L0X.h>

AWS_IOT hornbill;
VL53L0X sensor;
char WIFI_SSID[] = "hackathon2019";
char WIFI_PASSWORD[] = "fearlesscoder";
char HOST_ADDRESS[] = "a3kfcsv9dra7fl-ats.iot.us-west-2.amazonaws.com";
// CLIENT_ID must be unique to each thing
char CLIENT_ID[] = "client id 2";
// TOPIC_NAME must match MQTT path on AWS IoT
char TOPIC_NAME[] = "$aws/things/farm-gate-1/shadow/update";
int toasterPin = 2;

#define HIGH_SPEED

// Pin definitions
int myLed = 2;
int intPin = 4;

bool newData = false;

int distanceSensorReading;

uint32_t delt_t = 0, count = 0, sumCount = 0;  // used to control display output rate
float deltat = 0.0f, sum = 0.0f;          // integration interval for both filter schemes
uint32_t lastUpdate = 0, firstUpdate = 0; // used to calculate integration interval
uint32_t Now = 0;                         // used to calculate integration interval


int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad)
{
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}



void setup() {
  pinMode(toasterPin, OUTPUT);
  Serial.begin(115200);
  delay(2000);

  Serial.print("IN SETUP");

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

  if (hornbill.connect(HOST_ADDRESS, CLIENT_ID) == 0)
  {
    Serial.println("Connected to AWS");
    delay(1000);

    if (0 == hornbill.subscribe(TOPIC_NAME, mySubCallBackHandler))
    {
      Serial.println("Subscribe Successfull");

      Wire.begin(21, 22, 400000); // SDA (21), SCL (22) on ESP32, 400 kHz rate

      // Set up the led indicator
      pinMode(myLed, OUTPUT);
      digitalWrite(myLed, LOW);
      pinMode(intPin, INPUT);

      I2Cscan();

      delay(1000);

      sensor.init();
      sensor.setTimeout(500);

#if defined LONG_RANGE
      // lower the return signal rate limit (default is 0.25 MCPS)
      sensor.setSignalRateLimit(0.1);
      // increase laser pulse periods (defaults are 14 and 10 PCLKs)
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
      // reduce timing budget to 20 ms (default is about 33 ms)
      sensor.setMeasurementTimingBudget(20000);  // minimum timing budget 20 ms
#elif defined HIGH_ACCURACY
      // increase timing budget to 200 ms
      sensor.setMeasurementTimingBudget(200000);
#endif

      // Start continuous back-to-back mode (take readings as
      // fast as possible).  To use continuous timed mode
      // instead, provide a desired inter-measurement period in
      // ms (e.g. sensor.startContinuous(100)).
      sensor.startContinuous();

      attachInterrupt(intPin, myinthandler, FALLING);  // define interrupt for GPI01 pin output of VL53L0X

    }
    else
    {
      Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
      while (1);
    }
  }
  else
  {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }

  delay(2000);

}

void loop() {
  if (newData) // wait for data ready interrupt
  {
    newData = false; // reset data ready flag
    Now = micros(); // capture interrupt time
    // calculate time between last interrupt and current one, convert to sample data rate, and print to serial monitor
    Serial.print("data rate = "); Serial.print(1000000. / (Now - lastUpdate)); Serial.println(" Hz");

    distanceSensorReading = sensor.readRangeContinuousMillimeters();
    Serial.print(distanceSensorReading); // prit range in mm to serial monitor
    if (!(distanceSensorReading == 0 || distanceSensorReading == 8190)) {
      sprintf(payload, "{ \"state\": { \"desired\": {\"reading\":%d}}}", distanceSensorReading);
      if (hornbill.publish(TOPIC_NAME, payload) == 0)
      {
        Serial.print("Publish Message:");
        Serial.println(payload);
      }
      else
      {
        Serial.println("Publish failed");
      }
    }
    if (sensor.timeoutOccurred()) {
      Serial.print(" TIMEOUT");
    }

    Serial.println();
  }
  lastUpdate = Now;

  if (msgReceived == 1)
  {
    msgReceived = 0;
    Serial.print("Received Message:");
    Serial.println(rcvdPayload);

    digitalWrite(toasterPin, HIGH);
    delay(1000);
    digitalWrite(toasterPin, LOW);

  }
      vTaskDelay(500 / portTICK_RATE_MS);
  tick++;
}

void myinthandler()
{
  newData = true; // set the new data ready flag to true on interrupt
}

// I2C scan function
void I2Cscan()
{
  // scan for i2c devices
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

}
