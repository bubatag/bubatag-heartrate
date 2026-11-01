
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include <TinyGPSPlus.h>

// =====================================================
// CONFIGURACAO WI-FI
// =====================================================

const char* WIFI_SSID = "nome_da_rede";
const char* WIFI_PASSWORD = "senha_da_rede";

// =====================================================
// CONFIGURACAO BUBATAG
// =====================================================

const char* API_URL =
  "https://api-bubatag.onrender.com/api/telemetria/registro";

const char* API_HEALTH_URL =
  "https://api-bubatag.onrender.com/health";

const char* NUMERO_COLEIRA = "esp-coleira";

// =====================================================
// GPS NEO-6M
// =====================================================

TinyGPSPlus gps;
HardwareSerial SerialGPS(2);

const int GPS_RX_PIN = 16;
const int GPS_TX_PIN = 17;

const unsigned long GPS_TIMEOUT = 10000;

bool gpsDisponivel = false;

double latitude = 0.0;
double longitude = 0.0;

// =====================================================
// MAX30102
// =====================================================

MAX30105 particleSensor;

bool maxSensorDisponivel = false;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE] = {0};
byte rateSpot = 0;

long lastBeat = 0;

float beatsPerMinute = 0;
int beatAvg = 0;

bool batimentoValido = false;

unsigned long lastContactTime = 0;
unsigned long lastValidBeatTime = 0;

const unsigned long CONTACT_TIMEOUT = 2000;
const unsigned long BEAT_TIMEOUT = 5000;

const long IR_THRESHOLD = 10000;

// =====================================================
// DS18B20
// =====================================================

const int ONE_WIRE_BUS = 4;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temperaturaC = DEVICE_DISCONNECTED_C;
bool temperaturaValida = false;

// =====================================================
// INTERVALOS
// =====================================================

unsigned long lastTemperatureRequest = 0;
const unsigned long TEMPERATURE_INTERVAL = 5000;

unsigned long lastPrintTime = 0;
const unsigned long PRINT_INTERVAL = 5000;

unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 10000;

// =====================================================
// VARREDURA I2C
// =====================================================

void verificarDispositivosI2C() {

  Serial.println();
  Serial.println("Verificando dispositivos I2C...");

  byte dispositivosEncontrados = 0;

  for (byte endereco = 1; endereco < 127; endereco++) {

    Wire.beginTransmission(endereco);
    byte erro = Wire.endTransmission();

    if (erro == 0) {

      Serial.print("Dispositivo encontrado no endereco 0x");

      if (endereco < 16) {
        Serial.print("0");
      }

      Serial.println(endereco, HEX);

      dispositivosEncontrados++;
    }
  }

  if (dispositivosEncontrados == 0) {
    Serial.println("Nenhum dispositivo I2C encontrado.");
  }

  Serial.println("Fim da verificacao I2C.");
  Serial.println();
}

// =====================================================
// CONEXAO WI-FI
// =====================================================

void conectarWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.println("Conectando ao Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - inicio < 20000) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi conectado!");

    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("Nao foi possivel conectar ao Wi-Fi.");
  }
}

// =====================================================
// TESTE DE CONEXAO COM A API
// =====================================================

void testarConexaoAPI() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sem Wi-Fi. Teste da API cancelado.");
    return;
  }

  WiFiClientSecure cliente;
  cliente.setInsecure(); // Apenas para teste inicial.

  HTTPClient http;

  Serial.println("Testando conexao com a API...");

  if (http.begin(cliente, API_HEALTH_URL)) {

    http.setTimeout(20000);

    int resposta = http.GET();

    Serial.print("Status da API: ");
    Serial.println(resposta);

    if (resposta > 0) {
      Serial.println("Resposta da API:");
      Serial.println(http.getString());
    } else {
      Serial.print("Erro na conexao: ");
      Serial.println(http.errorToString(resposta));
    }

    http.end();

  } else {

    Serial.println("Falha ao iniciar conexao com a API.");
  }
}

// =====================================================
// INICIALIZACAO DO GPS NEO-6M
// =====================================================

void inicializarGPS() {

  SerialGPS.begin(
    9600,
    SERIAL_8N1,
    GPS_RX_PIN,
    GPS_TX_PIN
  );

  Serial.println("Comunicacao GPS preparada.");
  Serial.println("Aguardando posicao valida do NEO-6M.");
}

// =====================================================
// ATUALIZAR LOCALIZACAO GPS
// =====================================================

void atualizarGPS() {

  while (SerialGPS.available() > 0) {

    char caractere = SerialGPS.read();

    gps.encode(caractere);
  }

  if (gps.location.isValid() &&
      gps.location.age() <= GPS_TIMEOUT) {

    latitude = gps.location.lat();
    longitude = gps.location.lng();

    gpsDisponivel = true;

  } else {

    gpsDisponivel = false;
  }
}

// =====================================================
// INICIALIZACAO DO MAX30102
// =====================================================

void inicializarMAX30102() {

  Serial.println("Inicializando MAX30102...");

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {

    Serial.println("ERRO: MAX30102 nao encontrado!");
    Serial.println("Verifique a conexao I2C.");

    maxSensorDisponivel = false;
    return;
  }

  Serial.println("MAX30102 encontrado!");

  particleSensor.setup();

  particleSensor.setPulseAmplitudeRed(0x24);
  particleSensor.setPulseAmplitudeIR(0x24);
  particleSensor.setPulseAmplitudeGreen(0);

  maxSensorDisponivel = true;
}

// =====================================================
// LIMPAR LEITURAS DE BATIMENTO
// =====================================================

void limparBatimentos() {

  for (byte i = 0; i < RATE_SIZE; i++) {
    rates[i] = 0;
  }

  rateSpot = 0;
  beatAvg = 0;
  beatsPerMinute = 0;

  batimentoValido = false;
  lastBeat = 0;
  lastValidBeatTime = 0;
}

// =====================================================
// PROCESSAR LEITURA DO MAX30102
// =====================================================

void atualizarBatimentos(long irValue) {

  if (irValue < IR_THRESHOLD) {

    if (lastContactTime > 0 &&
        millis() - lastContactTime >= CONTACT_TIMEOUT) {

      if (batimentoValido) {
        Serial.println("Contato perdido. Limpando batimentos.");
      }

      limparBatimentos();
      lastContactTime = 0;
    }

    return;
  }

  lastContactTime = millis();

  if (checkForBeat(irValue)) {

    long agora = millis();

    if (lastBeat > 0) {

      long delta = agora - lastBeat;

      if (delta > 0) {

        beatsPerMinute = 60.0 / (delta / 1000.0);

        if (beatsPerMinute >= 20 &&
            beatsPerMinute <= 220) {

          rates[rateSpot++] = (byte)beatsPerMinute;
          rateSpot %= RATE_SIZE;

          int soma = 0;
          int quantidade = 0;

          for (byte i = 0; i < RATE_SIZE; i++) {

            if (rates[i] > 0) {
              soma += rates[i];
              quantidade++;
            }
          }

          if (quantidade > 0) {

            beatAvg = soma / quantidade;
            batimentoValido = true;
            lastValidBeatTime = agora;
          }
        }
      }
    }

    lastBeat = agora;
  }

  // Invalida o BPM se nao houver batimento recente.
  if (batimentoValido &&
      millis() - lastValidBeatTime > BEAT_TIMEOUT) {

    Serial.println("Batimento desatualizado. Aguardando nova leitura.");

    limparBatimentos();
  }
}

// =====================================================
// LEITURA DE TEMPERATURA
// =====================================================

void atualizarTemperatura() {

  float novaTemperatura = sensors.getTempCByIndex(0);

  Serial.print("Leitura DS18B20: ");
  Serial.print(novaTemperatura);
  Serial.println(" C");

  if (novaTemperatura == DEVICE_DISCONNECTED_C ||
      novaTemperatura < -55.0 ||
      novaTemperatura > 125.0) {

    temperaturaValida = false;

    Serial.println("Erro: sensor desconectado ou leitura invalida.");

  } else {

    temperaturaC = novaTemperatura;
    temperaturaValida = true;

    Serial.print("Temperatura atualizada: ");
    Serial.print(temperaturaC, 2);
    Serial.println(" C");
  }

  sensors.requestTemperatures();

  lastTemperatureRequest = millis();
}

// =====================================================
// ENVIO DE TELEMETRIA PARA A API
// =====================================================

void enviarTelemetria() {

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Sem Wi-Fi. Envio cancelado.");
    return;
  }

  if (!temperaturaValida) {
    Serial.println("Aguardando leitura valida de temperatura.");
    return;
  }

  // Nao registrar temperatura ambiente como corporal.
  if (temperaturaC < 34.0 || temperaturaC > 44.0) {

    Serial.println("Temperatura fora da faixa corporal.");
    Serial.println("Leitura mantida apenas para monitoramento local.");

    return;
  }

  if (!batimentoValido ||
      beatAvg < 20 ||
      beatAvg > 220 ||
      millis() - lastValidBeatTime > BEAT_TIMEOUT) {

    Serial.println("Aguardando batimento recente e valido.");
    return;
  }

  WiFiClientSecure cliente;
  cliente.setInsecure(); // Apenas para teste inicial.

  HTTPClient http;

  if (!http.begin(cliente, API_URL)) {
    Serial.println("Erro ao iniciar conexao HTTPS.");
    return;
  }

  http.setTimeout(20000);
  http.addHeader("Content-Type", "application/json");

  String json = "{";

  json += "\"n_coleira\":\"";
  json += NUMERO_COLEIRA;
  json += "\",";

  json += "\"temperatura\":";
  json += String(temperaturaC, 1);
  json += ",";

  json += "\"batimento_cardiaco\":";
  json += String(beatAvg);

  // Envia coordenadas somente quando houver GPS valido.
  if (gpsDisponivel) {

    json += ",\"latitude\":";
    json += String(latitude, 6);

    json += ",\"longitude\":";
    json += String(longitude, 6);
  }

  json += "}";

  Serial.println("Enviando telemetria...");
  Serial.println(json);

  if (gpsDisponivel) {

    Serial.print("Latitude: ");
    Serial.println(latitude, 6);

    Serial.print("Longitude: ");
    Serial.println(longitude, 6);

  } else {

    Serial.println("GPS sem posicao valida. Enviando sem coordenadas.");
  }

  int codigoResposta = http.POST(json);

  if (codigoResposta > 0) {

    Serial.print("Resposta HTTP: ");
    Serial.println(codigoResposta);

    Serial.println("Resposta do servidor:");
    Serial.println(http.getString());

  } else {

    Serial.print("Erro no envio: ");
    Serial.println(http.errorToString(codigoResposta));
  }

  http.end();
}

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==========================================");
  Serial.println(" BUBATAG - MONITOR BIOMETRICO");
  Serial.println("==========================================");

  Wire.begin(21, 22);

  // Inicializa a comunicacao do GPS.
  inicializarGPS();

  verificarDispositivosI2C();

  conectarWiFi();

  testarConexaoAPI();

  inicializarMAX30102();

  sensors.begin();
  sensors.setWaitForConversion(false);

  sensors.requestTemperatures();
  lastTemperatureRequest = millis();

  Serial.println("DS18B20 inicializado!");

  Serial.println("------------------------------------------");

  if (maxSensorDisponivel) {
    Serial.println("MAX30102: Disponivel");
  } else {
    Serial.println("MAX30102: Indisponivel");
  }

  Serial.println("DS18B20: Inicializado");
  Serial.println("GPS: Preparado");
  Serial.println("Sistema iniciado!");
  Serial.println("------------------------------------------");
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // -------------------------------------------------
  // 1. GPS
  // -------------------------------------------------

  atualizarGPS();

  // -------------------------------------------------
  // 2. MAX30102
  // -------------------------------------------------

  long irValue = 0;

  if (maxSensorDisponivel) {

    irValue = particleSensor.getIR();

    atualizarBatimentos(irValue);
  }

  // -------------------------------------------------
  // 3. DS18B20
  // -------------------------------------------------

  if (millis() - lastTemperatureRequest >=
      TEMPERATURE_INTERVAL) {

    atualizarTemperatura();
  }

  // -------------------------------------------------
  // 4. MONITOR SERIAL
  // -------------------------------------------------

  if (millis() - lastPrintTime >= PRINT_INTERVAL) {

    lastPrintTime = millis();

    Serial.println("------------------------------------------");

    if (maxSensorDisponivel) {

      Serial.print("IR: ");
      Serial.println(irValue);

      if (irValue < IR_THRESHOLD) {

        Serial.println("Batimento: Aguardando contato...");

      } else if (batimentoValido) {

        Serial.print("Batimento medio: ");
        Serial.print(beatAvg);
        Serial.println(" BPM");

      } else {

        Serial.println("Batimento: Calculando...");
      }

    } else {

      Serial.println("MAX30102: Sensor indisponivel.");
    }

    Serial.print("Temperatura: ");

    if (!temperaturaValida) {

      Serial.println("Sem leitura valida");

    } else {

      Serial.print(temperaturaC, 2);
      Serial.println(" C");

      if (temperaturaC < 34.0 || temperaturaC > 44.0) {
        Serial.println("Leitura fora da faixa corporal.");
      }
    }

    // Situacao do GPS.
    Serial.print("GPS: ");

    if (gpsDisponivel) {

      Serial.println("Posicao valida");

      Serial.print("Latitude: ");
      Serial.println(latitude, 6);

      Serial.print("Longitude: ");
      Serial.println(longitude, 6);

    } else {

      Serial.println("Aguardando sinal ou modulo GPS.");
    }

    Serial.print("Wi-Fi: ");

    Serial.println(
      WiFi.status() == WL_CONNECTED
      ? "Conectado"
      : "Desconectado"
    );

    Serial.println("------------------------------------------");
  }

  // -------------------------------------------------
  // 5. ENVIO PARA O BUBATAG
  // -------------------------------------------------

  if (millis() - lastSendTime >= SEND_INTERVAL) {

    lastSendTime = millis();

    enviarTelemetria();
  }
}
