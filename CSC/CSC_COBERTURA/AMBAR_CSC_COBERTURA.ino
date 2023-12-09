// ESP32 DEV KIT - AMBAR - Cliente Estacoes Climaticas
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Wire.h>
#include <Adafruit_GFX.h>      //https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>  //https://github.com/adafruit/Adafruit_SSD1306
#include <LiquidCrystal_I2C.h>
#include "Adafruit_SHT31.h"  //https://github.com/adafruit/Adafruit_SHT31/archive/master.zip

// Driver
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// iP for OTA: 192.168.1.16
// 30/11/2023
// WiFi
const char* host = "AMBAR 1 - CEC - COBERTURA";
const char* SSID = "Guilherme OiFibra 2G";
const char* PASS = "zezewifi2805";
//const char* SSID ="Galaxy A13 0109";
//const char* PASS ="njji1312";
//const char* SSID ="UNB Wireless";
//const char* PASS ="";
const char* mqtt_server = "broker.hivemq.com";  // MQTT server eh o mesmo que "broker"? Sim

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

LiquidCrystal_I2C lcd(0x27, 16, 2);  // O endereco de fabrica do LCD geralmente é 0x27 ou 0x7F

// Variaveis
volatile uint8_t flag_msgFromPayload = 0;
volatile uint8_t send = 0;
long lastTemp = 0;
float temperatura;
float umidade;
float hic;
char temp[8];
char umid[8];
char heatindex[8];

// Objetos
WiFiClient espClient;
PubSubClient client(espClient);

WebServer server(80);  // Webserver para se comunicar via browser com ESP32

/* Códigos da página que será aberta no browser 
   (quando comunicar via browser com o ESP32) 
   Esta página exigirá um login e senha, de modo que somente 
   quem tenha estas informações consiga atualizar o firmware
   do ESP32 de forma OTA */
const char* loginIndex =
  "<form name='loginForm'>"
  "<table width='20%' bgcolor='A09F9F' align='center'>"
  "<tr>"
  "<td colspan=2>"
  "<center><font size=4><b>AMBAR 1 - CEC Cobertura - identifique-se</b></font></center>"
  "<br>"
  "</td>"
  "<br>"
  "<br>"
  "</tr>"
  "<td>Login:</td>"
  "<td><input type='text' size=25 name='userid'><br></td>"
  "</tr>"
  "<br>"
  "<br>"
  "<tr>"
  "<td>Senha:</td>"
  "<td><input type='Password' size=25 name='pwd'><br></td>"
  "<br>"
  "<br>"
  "</tr>"
  "<tr>"
  "<td><input type='submit' onclick='check(this.form)' value='Acessar'></td>"
  "</tr>"
  "</table>"
  "</form>"
  "<script>"
  "function check(form)"
  "{"
  "if(form.userid.value=='admin' && form.pwd.value=='admin')"
  "{"
  "window.open('/serverIndex')"
  "}"
  "else"
  "{"
  " alert('Login ou senha inválidos')"
  "}"
  "}"
  "</script>";

const char* serverIndex =
  "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "<input type='file' name='update'>"
  "<input type='submit' value='Update'>"
  "</form>"
  "<div id='prg'>Progresso: 0%</div>"
  "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('Progresso: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('Sucesso!')"
  "},"
  "error: function (a, b, c) {"
  "}"
  "});"
  "});"
  "</script>";

void setup() {

  Serial.begin(115200);
  setup_wifi();
  setup_server();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  lcd.init();
  lcd.setBacklight(HIGH);  // Luz de fundo acesa
  lcd.setCursor(0, 0);
  Serial.println("SHT31 test");
  if (!sht31.begin(0x44)) {  // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  temperatura = sht31.readTemperature();
  umidade = sht31.readHumidity();
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
  Serial.println();

  // display temperature
  lcd.setCursor(9, 0);  // Coluna, Linha
  lcd.print("Temperatura: ");
  lcd.print(temperatura);

  // display humidity
  lcd.setCursor(9, 4);  // Coluna, Linha
  lcd.print(umidade);
  lcd.print("%");
  dtostrf(temperatura, 3, 1, temp);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  dtostrf(umidade, 3, 1, umid);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
                                     // dtostrf(hic, 3, 1, computeHeatIndex);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
  client.publish("ambar/climaCobertura/temp", temp);
  client.publish("ambar/climaCobertura/umidade", umid);
}
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectado a rede: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}
void setup_server() {
  /* Usa MDNS para resolver o DNS */
  if (!MDNS.begin(host)) {
    //http://esp32.local
    Serial.println("Erro ao configurar mDNS. O ESP32 vai reiniciar em 1s...");
    delay(1000);
    ESP.restart();
  }

  Serial.println("mDNS configurado e inicializado;");

  /* Configfura as páginas de login e upload de firmware OTA */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });

  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });

  /* Define tratamentos do update de firmware OTA */
  server.on(
    "/update", HTTP_POST, []() {
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    []() {
      HTTPUpload& upload = server.upload();

      if (upload.status == UPLOAD_FILE_START) {
        /* Inicio do upload de firmware OTA */
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          Update.printError(Serial);
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* Escrevendo firmware enviado na flash do ESP32 */
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          Update.printError(Serial);
      } else if (upload.status == UPLOAD_FILE_END) {
        /* Final de upload */
        if (Update.end(true))
          Serial.printf("Sucesso no update de firmware: %u\nReiniciando ESP32...\n", upload.totalSize);
        else
          Update.printError(Serial);
      }
    });
  server.begin();
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...\n\r");

    if (client.connect("espClient")) {
      Serial.println("Conectado");
      if(send)
      {
      send = 0;
      client.publish("ambar/climaCobertura/temp", temp);
      client.publish("ambar/climaCobertura/umidade", umid);
      }

    } else {
      Serial.print("Erro:");
      Serial.print(client.state());
      Serial.println(" reconectando em 5 segundos");
      delay(5000);
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

  server.handleClient();  //  "it handles clients. it should be called in loop. it calls the functions set with server.on()"

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - lastTemp > 60000) {
    lastTemp = millis();
    
    volatile static int min = 5;
    if (min-- <= 0) {
      min = 5;
      send ^= 1;
      temperatura = sht31.readTemperature();
      umidade = sht31.readHumidity();
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
      Serial.println();

      // display temperature
      lcd.setCursor(9, 0);  // Coluna, Linha
      lcd.print("Temperatura: ");
      lcd.print(temperatura);

      // display humidity
      lcd.setCursor(9, 4);  // Coluna, Linha
      lcd.print(umidade);
      lcd.print("%");
      dtostrf(temperatura, 3, 1, temp);  // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      dtostrf(umidade, 3, 1, umid);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
                                         // dtostrf(hic, 3, 1, computeHeatIndex);      // Converte ponto flutuante em string. (float_value, min_width, decimal_points, where_to_store)
      client.publish("ambar/climaCobertura/temp", temp);
      client.publish("ambar/climaCobertura/umidade", umid);
      //client.publish("ambar/climaCobertura/heatindex", heatindex);
      //  hic = dht.computeHeatIndex(temperatura, umidade, false);          // Indice de calor
    }
  }
}