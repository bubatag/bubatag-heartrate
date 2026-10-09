#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

float beatsPerMinute;
int beatAvg;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 5000; 

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando BUBATAG - Monitor Cardíaco...");

  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("Sensor MAX30102 não foi encontrado. Verifique as conexões!");
    while (1);
  }

  particleSensor.setup(); 
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0x00);
}

void loop() {
  long irValue = particleSensor.getIR();

  // Verifica batimento continuamente
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  // Filtro de tempo
  if (millis() - lastPrintTime >= PRINT_INTERVAL) {
    lastPrintTime = millis();

    if (irValue < 50000) {
      Serial.println("Aguardando contato / Coloque o dedo no sensor...");
    } else {
      Serial.print("BPM Instantâneo: ");
      Serial.print(beatsPerMinute, 1);
      Serial.print(" | Média BPM: ");
      Serial.println(beatAvg);
    }
  }
}