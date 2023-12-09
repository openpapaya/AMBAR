// ESP32 DEV KIT - AMBAR - Cliente Estacoes Climaticas - VARANDA
#include <Arduino.h>
#include <stdio.h>

#include <string.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <PubSubClient.h>
#include <mqtt_client.h>
#include <Wire.h>
#include "DHT.h" /* Biblioteca DHT- github.com/adafruit/DHT-sensor-library */
#include <LiquidCrystal_I2C.h>


#include <stdio.h>
#include <stdint.h>
#include <string.h>

// iP for OTA: 192.168.1.5
// 30/11/2023
// WiFi
//const char* host = "AMBAR 1 - CEC - VARANDA";
const char* ssid = "Guilherme OiFibra 2G";
const char* pass = "zezewifi2805";
//const char* SSID ="Galaxy A13 0109";
//const char* PASS ="njji1312";
//const char* SSID ="UNB Wireless";
//const char* PASS ="";
const char* mqtt_server = "broker.hivemq.com";  //

// Defining ESP32 pins
#define DHTPIN 4
#define LED 13
#define GPIO14 14
#define RXD2 16
#define TXD2 17
#define GPIO18 18
#define GPIO19 19
#define I2C_SDA 21
#define I2C_SCL 22
#define GPIO23 23
#define GPIO25 25
#define GPIO26 26
#define GPIO27 27
#define GPIO32 32
#define GPIO33 33

/*-------- Configurações DHT ----------- */
#define DHTTYPE DHT22     /* Define o modelo DHT 22, AM2302, AM2321 */
DHT dht(DHTPIN, DHTTYPE); /* DHT (pino,tipo) */

LiquidCrystal_I2C lcd(0x26, 16, 2);  // O endereco de fabrica do LCD geralmente é 0x27 ou 0x7F

// Variaveis
volatile uint8_t flag_msgFromPayload = 0;
volatile uint8_t send = 0;
long lastTemp = 0;

float temperatura;
float umidade;
float hic;
int tLCD;
int uLCD;
int icLCD;
char temp[8];
char umid[8];
char heatIndex[8];
// Objetos
WiFiClient espClient;
PubSubClient client(espClient);

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "AMBAR CEC - VARANDA 2");
  });

  AsyncElegantOTA.begin(&server);  // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  // -----------------
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  lcd.init();
  lcd.setBacklight(HIGH);  // Luz de fundo acesa
  lcd.print("CLIMA VARANDA:  ");
  dht.begin(); /* Comunicação DHT */
  delay(2000);
  // display temperature

  temperatura = dht.readTemperature();                      /* leitura de Temperatura */
  umidade = dht.readHumidity();                             /* leitura de Umidade */
  hic = dht.computeHeatIndex(temperatura, umidade, false);  // Indice de calor

  dtostrf(temperatura, 2, 1, temp);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  dtostrf(umidade, 2, 0, umid);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  dtostrf(hic, 2, 1, heatIndex);     // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  tLCD = temperatura;
  lcd.setCursor(0, 4);  // Coluna, Linha
  lcd.print(temp);
  lcd.setCursor(4, 4);  // Coluna, Linha
  lcd.print("C ");
  // display humidity
  uLCD = umidade;
  lcd.setCursor(6, 4);  // Coluna, Linha
  lcd.print(umid);
  lcd.setCursor(8, 4);  // Coluna, Linha
  lcd.print("% ");

  // Display heat index
  icLCD = hic;
  lcd.setCursor(12, 4);  // Coluna, Linha
  lcd.print(heatIndex);

  client.publish("ambar/climaVaranda2/temp", temp);
  client.publish("ambar/climaVaranda2/umidade", umid);
  client.publish("ambar/climaVaranda2/hic", heatIndex);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...\n\r");

    if (client.connect("esp32Client")) {
       Serial.println("Conectado");
      if(send)
      {
        send = 0;
      client.publish("ambar/climaVaranda2/temp", temp);
      client.publish("ambar/climanVaranda2/umidade", umid);
      client.publish("ambar/climanVaranda2/hic", heatIndex);
      }
    } else {
      Serial.print("Erro:");
      Serial.print(client.state());
      Serial.println(" reconectando em 5 segundos");
      // Colocar para conectar pelo celular
      //const char* ssid = "Galaxy A13 0109";
      //const char* pass = "njji1312";
      delay(3000);
    }
  }
}
// ------------------------- MQTT --------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\n\r ");
  Serial.print("Menssagem recebida[");
  Serial.print(topic);
  Serial.print("]\n\r ");
  // Convert the payload byte array to a string
  String receivedPayload = "";
  String topicStr = topic;

  for (unsigned int i = 0; i < length; i++) {
    // Process the received payload
    Serial.print((char)payload[i]);  // Imprime no terminal o payload do topic
    receivedPayload += (char)payload[i];
    flag_msgFromPayload = 1;
  }
  //  Serial.print("Received payload: ");
  //  Serial.println(receivedPayload);
}

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (millis() - lastTemp > 60000) {
    lastTemp = millis();
    volatile static int min = 5;
    if (min-- <= 0) {
      min = 5;
      send = 1;
      temperatura = dht.readTemperature();                      /* leitura de Temperatura */
      umidade = dht.readHumidity();                             /* leitura de Umidade */
      hic = dht.computeHeatIndex(temperatura, umidade, false);  // Indice de calor


      if (!isnan(temperatura)) {  // check if 'is not a number'
        Serial.print("Temp *C = ");
        Serial.println(temperatura);
      } else {
        temperatura = 0.0;
        Serial.println("Falhou em ler a temperatura");
      }

      if (!isnan(umidade)) {  // check if 'is not a number'
        Serial.print("Hum. % = ");
        Serial.println(umidade);
      } else {
        umidade = 0.0;
        Serial.println("Falhou em ler a umidade");
      }

      // ***********************************************************************************************************
      // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
      //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
      //
      //              (" C    L    I    M    A         V    A    R    A    N    D    A    :           ");
      //              (" X    X    ,    X    C         X    X    %         I    C    X    X           ");
      // ***********************************************************************************************************
      // display temperature
      dtostrf(temperatura, 2, 1, temp);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      dtostrf(umidade, 2, 0, umid);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      dtostrf(hic, 2, 1, heatIndex);     // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      tLCD = temperatura;
      lcd.setCursor(0, 4);  // Coluna, Linha
      lcd.print(temp);
      lcd.setCursor(4, 4);  // Coluna, Linha
      lcd.print("C ");
      // display humidity
      uLCD = umidade;
      lcd.setCursor(6, 4);  // Coluna, Linha
      lcd.print(umid);
      lcd.setCursor(8, 4);  // Coluna, Linha
      lcd.print("% ");

      // Display heat index
      icLCD = hic;
      lcd.setCursor(12, 4);  // Coluna, Linha
      lcd.print(heatIndex);

      client.publish("ambar/climaVaranda2/temp", temp);
      client.publish("ambar/climaVaranda2/umidade", umid);
      client.publish("ambar/climaVaranda2/hic", heatIndex);
    }
  }
}