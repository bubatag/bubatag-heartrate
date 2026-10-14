#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <OneWire.h>
#include <DallasTemperature.h>

// =====================================================
// MAX30102
// =====================================================

MAX30105 particleSensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;

long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;


// =====================================================
// DS18B20
// =====================================================

const int ONE_WIRE_BUS = 4;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperaturaC = 0;


// =====================================================
// TIMERS
// =====================================================

unsigned long lastTemperatureRequest = 0;
const unsigned long TEMPERATURE_INTERVAL = 5000;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 5000;


// =====================================================
// SETUP
// =====================================================

void setup() {

    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==========================================");
    Serial.println("   BUBATAG - Monitor Biométrico Completo");
    Serial.println("==========================================");


    // =================================================
    // I2C - MAX30102
    // =================================================

    Wire.begin(21, 22);

    Serial.println("Inicializando MAX30102...");

    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {

        Serial.println("ERRO: MAX30102 nao encontrado!");

        while (1) {
            delay(1000);
        }
    }

    Serial.println("MAX30102 encontrado!");


    // Configuração do MAX30102
    particleSensor.setup();

    particleSensor.setPulseAmplitudeRed(0x24);
    particleSensor.setPulseAmplitudeIR(0x24);
    particleSensor.setPulseAmplitudeGreen(0);


    // =================================================
    // DS18B20
    // =================================================

    sensors.begin();

    // IMPORTANTE:
    // Não deixar o DS18B20 bloquear o loop
    sensors.setWaitForConversion(false);

    // Faz a primeira solicitação
    sensors.requestTemperatures();

    lastTemperatureRequest = millis();


    Serial.println("DS18B20 inicializado!");
    Serial.println("Sistema pronto!");

    Serial.println();
}


// =====================================================
// LOOP
// =====================================================

void loop() {


    // =================================================
    // 1. MAX30102
    // =================================================
    // Esta parte roda continuamente.
    // NÃO deve ter delays grandes aqui.

    long irValue = particleSensor.getIR();


    if (checkForBeat(irValue)) {

        long delta = millis() - lastBeat;

        lastBeat = millis();

        beatsPerMinute = 60.0 / (delta / 1000.0);


        if (beatsPerMinute > 20 && beatsPerMinute < 255) {

            rates[rateSpot++] = (byte)beatsPerMinute;

            rateSpot %= RATE_SIZE;


            beatAvg = 0;

            for (byte x = 0; x < RATE_SIZE; x++) {
                beatAvg += rates[x];
            }

            beatAvg /= RATE_SIZE;
        }
    }


    // =================================================
    // 2. DS18B20
    // =================================================
    // Faz a solicitação apenas uma vez por segundo.

    if (millis() - lastTemperatureRequest >= TEMPERATURE_INTERVAL) {

        // Pega o resultado da conversão anterior
        float novaTemperatura = sensors.getTempCByIndex(0);

        if (novaTemperatura != DEVICE_DISCONNECTED_C) {
            temperaturaC = novaTemperatura;
        }

        // Inicia uma nova conversão
        sensors.requestTemperatures();

        lastTemperatureRequest = millis();
    }


    // =================================================
    // 3. SERIAL
    // =================================================

    if (millis() - lastPrintTime >= PRINT_INTERVAL) {

        lastPrintTime = millis();

        Serial.println("------------------------------------------");


        // -------------------------
        // Batimento
        // -------------------------

        Serial.print("IR: ");
        Serial.println(irValue);


        if (irValue < 10000) {

            Serial.println("Batimento Cardiaco: Aguardando contato...");

        } else {

            Serial.print("Batimento Cardiaco: ");

            if (beatsPerMinute > 20 && beatsPerMinute < 255) {

                Serial.print(beatsPerMinute, 1);
                Serial.print(" BPM");

            } else {

                Serial.print("Calculando...");

            }

            Serial.print(" (Media: ");
            Serial.print(beatAvg);
            Serial.println(" BPM)");
        }


        // -------------------------
        // Temperatura
        // -------------------------

        Serial.print("Temperatura Corporal: ");

        if (temperaturaC == DEVICE_DISCONNECTED_C) {

            Serial.println("Erro - sensor desconectado");

        } else {

            Serial.print(temperaturaC, 1);
            Serial.println(" C");
        }


        Serial.println("------------------------------------------");
    }
}