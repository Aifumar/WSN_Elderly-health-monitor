#include "painlessMesh.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#define MESH_PREFIX "mesh_network"
#define MESH_PASSWORD "password"
#define MESH_PORT 5555

Scheduler userScheduler;
painlessMesh  mesh;
MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
String pesan;
boolean led = 0;
String namaNode = "Aif"; // Nama node 

void sendMessage() ;

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );

void sendMessage() {
  mesh.sendBroadcast( pesan );
  led = !led;
  digitalWrite(LED_BUILTIN, led);
  taskSendMessage.setInterval( TASK_SECOND * 5 );
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("Initializing...");
  mesh.setDebugMsgTypes( ERROR | STARTUP );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  particleSensor.enableDIETEMPRDY();
}

void loop() {
  
  long irValue = particleSensor.getIR();
  float temperature = particleSensor.readTemperature();

  if (checkForBeat(irValue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  Serial.print(", TempC=");
  Serial.print(temperature);

  if (irValue < 50000) {
    Serial.print(" No finger?");
    pesan = (namaNode + " => Alat dilepas");
  } else if ((beatAvg > 170) || (beatAvg < 50)){
    pesan = (namaNode + " => ⚠ BAHAYA ⚠ 💓: "+String(beatAvg)+"(BPM)\t🌡️: "+String(temperature)+"(°C)");
  } else {
    pesan = (namaNode + " => 💓: "+String(beatAvg)+"(BPM)\t🌡️: "+String(temperature)+"(°C)");
  }
  Serial.println();
  mesh.update();
}
