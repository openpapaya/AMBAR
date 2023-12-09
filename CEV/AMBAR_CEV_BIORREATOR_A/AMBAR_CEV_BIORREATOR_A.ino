// ESP32 DEV KIT - AMBAR - Cliente Estacao de Compostagem e Vermicompostagem
#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <PubSubClient.h>
#include <mqtt_client.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// Endereco DS18B20 1: 28FF641E837C5D30
// Endereco DS18B20 2: 28FF641E8377FA3F
// iP for OTA: 192.168.1.5/

// Wifi
//const char* host = "AMBAR 1 - CEV - BIORREATOR A";
const char* ssid = "Guilherme OiFibra 2G";
const char* pass = "zezewifi2805";
//const char* SSID = "Galaxy A13 0109";
//const char* PASS = "ypvs5881";
//const char* SSID ="UNB Wireless";
//const char* PASS ="";
const char* mqtt_server = "broker.hivemq.com";  // 

// Defining ESP32 pins
#define GPIO12 12
#define DHTPIN 13
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
#define TIP1 0b11111100
#define TIP2 0b11111110
#define TIP3 0b11110111
#define TIP4 0b11110011
#define TIP5 0b11101110
#define TIP6 0b11101100
#define TIP7 0b11010111
#define TIP8 0b11010011
// Variaveis
volatile uint8_t flag_msgFromPayload = 0;
float temp[2];  // String para guardar temperaturas
char tempTopstr[8];
char tempCenterstr[8];
char tempBottomstr[8];
// String para guardar os valores tipicos de compostagem
int numberOfDevices;  // Numero de dispositivos One Wire encontrados
const int oneWireBus = 5;
long lastTemp = 0;
long lastTemp3Sec = 0;
long lcd_refresh = 0;
// Objetos
OneWire oneWire(oneWireBus);          // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature sensor
DeviceAddress tempDeviceAddress;      // We'll use this variable to store a found device address
WiFiClient espClient;
PubSubClient client(espClient);
AsyncWebServer server(80);
//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);  // O endereco de fabrica do LCD geralmente é 0x27 ou 0x7F

void setup(void) {

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
    request->send(200, "text/plain", "AMBAR CEV - BIORREATOR A");
  });

  AsyncElegantOTA.begin(&server);  // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  // -----------------
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  sensors.begin();  // 1 - Start the DS18B20 sensor
  oneWireSearch();
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.setBacklight(HIGH);  // Luz de fundo acesa
  lcd.setCursor(0, 0);
  // ***********************************************************************************************************
  // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
  //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
  //
  //              ("           T    E    M    P    E    R    A    T    U    R    A:               ");
  //              (" X    X    ,    X    |    X    X    ,    X    |    X    X     ,   X           ");
  // ***********************************************************************************************************
  lcd.print("  TEMPERATURA:  ");
  sensors.requestTemperatures();
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      temp[i] = sensors.getTempC(tempDeviceAddress);
    }
  }
  dtostrf(temp[0], 2, 1, tempCenterstr);     // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  dtostrf(temp[1], 2, 1, tempTopstr);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  client.publish("ambar/CEV/temp_reatorA/top", tempTopstr);
  client.publish("ambar/CEV/temp_reatorA/center", tempCenterstr);
  lcd.setCursor(0, 4);  // Coluna, Linha
  lcd.print(" ");
  //lcd.setCursor(6, 4);  // Coluna, Linha
  lcd.print(tempTopstr);
  lcd.setCursor(6, 4);  // Coluna, Linha
  //lcd.print("      ");
  lcd.print(tempCenterstr);
  Wire.beginTransmission(0x20);  // transmit to device #4
  Wire.write(0b11111111);        // Desabilita o decoder
  Wire.endTransmission();        // Stop transmitting
}

void oneWireSearch() {  // 1.1 - Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  // 1.2 - locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
  // 1.3 - Loop through each device, print out address
  for (int i = 0; i < numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...\n\r");

    if (client.connect("espClient")) {
      Serial.println("Conectado");
      client.subscribe("ambar/CEV/reatorA/airpump");
      client.publish("ambar/CEV/temp_reatorA/top", tempTopstr);
      client.publish("ambar/CEV/temp_reatorA/center", tempCenterstr);
    } else {
      // Serial.print("Erro:");
      // Serial.print(client.state());
      Serial.println(" reconectando em 5 segundos");
      // Colocar para conectar pelo celular
      //const char* ssid = "Galaxy A13 0109";
      //const char* pass = "njji1312";
      delay(500);
    }
  }
}

// ------------------------- MQTT --------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\n\r ");
  Serial.print("Menssagem recebida!");
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
  if (topicStr == "ambar/CEV/reatorA/airpump") {
    if (payload[0] == '1')  //
    {
      //  PINOS: SN74HC139(PCF8574)
      //  P7 P6 P5 P4 P3 P2 P1 P0
      //            0        0  0  -> t6, t6
      //            0        1  0  -> t5, t5
      //            1        0  0  -> t1, t1
      //            1        1  0  -> t2, t2
      //  PINOS: SN74HC139(PCF8574)
      //  P7 P6 P5 P4 P3 P2 P1 P0
      //         0     0  0        ->
      //         0     0  1        ->
      //         1     0  0        ->
      //         1     0  1        ->
      Wire.beginTransmission(0x20);  // transmit to device #4
      //           76543210
      //           xx5xE2xx
      Wire.write(TIP1);        // Liga a bomba de ar
      Wire.endTransmission();  // stop transmitting
      Serial.print("\n\r ");

      Serial.print("Bomba de ar ligada");
      Serial.print(topic);
      Serial.print("\n\r ");
      // ***********************************************************************************************************
      // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
      //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
      //
      //              ("           T    E    M    P    E    R    A    T    U    R    A:               ");
      //              (" X    X    ,    X    |    X    X    ,    X    |    X    X     ,   X           ");
      // ***********************************************************************************************************
      lcd.setCursor(0, 0);  // Coluna, Linha
                            //          "xxxxxxxxxxxxxxxx"
      lcd.print("  Air pump: ON  ");
      lcd.setCursor(0, 4);  // Coluna, Linha
      lcd.print("Temp:           ");
      lcd.setCursor(5, 4);  // Coluna, Linha
      lcd.print(tempCenterstr);
    }
    if (payload[0] == '0')  //
    {
      Wire.beginTransmission(0x20);  // transmit to device #4
      Wire.write(0b11111111);        // Desabilita o decoder
      Wire.endTransmission();        // Stop transmitting
      Serial.print("\n\r ");
      Serial.print("Bomba de ar desligada");
      Serial.print(topic);
      Serial.print("\n\r ");
      lcd.setCursor(0, 0);  // Coluna, Linha
                            //          "xxxxxxxxxxxxxxxx"
      lcd.print("  TEMPERATURA:  ");
      lcd.setCursor(0, 4);  // Coluna, Linha
      lcd.print(" ");
      //lcd.setCursor(6, 4);  // Coluna, Linha
      lcd.print(tempTopstr);
      lcd.setCursor(7, 4);  // Coluna, Linha
                            // lcd.print("      ");
      lcd.print(tempCenterstr);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (millis() - lastTemp > 10000) {  // A função millis() retorna um número indicando há quantos milissegundos o Arduino está ligado.
    lastTemp = millis();              // Atualiza o node-red a cada 5 segundos
    volatile static int tresMin = 3;
    if (tresMin-- <= 0) {
      tresMin = 3;
      lcd.init();
      lcd.setBacklight(HIGH);  // Luz de fundo acesa
      lcd.setCursor(0, 0);
      lcd.print("  TEMPERATURA:  ");
      sensors.requestTemperatures();
      for (int i = 0; i < numberOfDevices; i++) {
        // Search the wire for address
        if (sensors.getAddress(tempDeviceAddress, i)) {
          // Output the device ID
          //Serial.print("Temperature for device: ");
          // Serial.println(i, DEC);
          // Print the data
          temp[i] = sensors.getTempC(tempDeviceAddress);
          // Serial.println("Temperatura centro: ");
          // Serial.println(temp[0]);
          // Serial.println("ºC");
        }
        dtostrf(temp[0], 2, 1, tempCenterstr );     // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
        dtostrf(temp[1], 2, 1, tempTopstr);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      }
      client.publish("ambar/CEV/temp_reatorA/top", tempTopstr);
      client.publish("ambar/CEV/temp_reatorA/center", tempCenterstr);
    }
  }
  if (millis() - lastTemp3Sec > 3000) {  // A função millis() retorna um número indicando há quantos milissegundos o Arduino está ligado.
    lastTemp3Sec = millis();
    sensors.requestTemperatures();
    for (int i = 0; i < numberOfDevices; i++) {
      // Search the wire for address
      if (sensors.getAddress(tempDeviceAddress, i)) {
        temp[i] = sensors.getTempC(tempDeviceAddress);
      }
    }
    // Output the device ID
    //Serial.print("Temperature for device: ");
    // Serial.println(i, DEC);
    // Print the data
    // Serial.println("Temperatura centro: ");
    // Serial.println(temp[0]);
    // Serial.println("ºC");

    dtostrf(temp[0], 2, 1, tempCenterstr);     // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
    dtostrf(temp[1], 2, 1, tempTopstr);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
    lcd.setCursor(0, 4);                    // Coluna, Linha
    lcd.print(" ");
    //lcd.setCursor(6, 4);  // Coluna, Linha
    lcd.print(tempTopstr);
    lcd.setCursor(9, 4);  // Coluna, Linha
    lcd.print(" ");
    lcd.print(tempCenterstr);
  }
  // ***********************************************************************************************************
  // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
  //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
  //
  //              ("           T    E    M    P    E    R    A    T    U    R    A:               ");
  //              ("      X    X    ,    X    |    X    X    ,    X    |    X    X     ,   X      ");
  // ***********************************************************************************************************
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}