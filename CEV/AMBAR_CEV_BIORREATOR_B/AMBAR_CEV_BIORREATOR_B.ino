// ESP32 DEV KIT - AMBAR - Cliente Estacao de Compostagem e Vermicompostagem
#include <stdio.h>
#include <stdint.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

// Endereco DS18B20 1: 28FF641E837C5D30
// Endereco DS18B20 2: 28FF641E8377FA3F
// iP for OTA: 192.168.1.11/

// Wifi
const char* host = "AMBAR 1 - CEV - BIORREATOR B";
const char* SSID = "Guilherme OiFibra 2G";
const char* PASS = "zezewifi2805";
//const char* ssid = "Galaxy A13 0109";
//const char* pass = "njji1312";
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

// Variaveis
volatile uint8_t flag_msgFromPayload = 0;
float temp[2];        // String para guardar temperaturas
int numberOfDevices;  // Numero de dispositivos One Wire encontrados
const int oneWireBus = 5;
long lastTemp = 0;

// Objetos
OneWire oneWire(oneWireBus);          // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);  // Pass our oneWire reference to Dallas Temperature sensor
// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;
WiFiClient espClient;
PubSubClient client(espClient);
WebServer server(80);  //~Webserver para se comunicar via browser com ESP32
//Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);  // O endereco de fabrica do LCD geralmente é 0x27 ou 0x7F

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
  "<center><font size=4><b>AMBAR 1 - CEV - BIORREATOR A - identifique-se</b></font></center>"
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

void setup(void) {

  Serial.begin(115200);
  setup_wifi();
  setup_server();
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
    //              ("      X    X    ,    X    |    X    X    ,    X    |    X    X     ,   X      ");
    // ***********************************************************************************************************
  lcd.print("  TEMPERATURA:   ");
  lcd.print(" XX,X|XX,X|XX,X  ");
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
  delay(5000);
}
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...\n\r");
    client.publish("ambar/CEV/temp_reatorB", "Conectando");

    if (client.connect("AMBAR1")) {
      Serial.println("Conectado");

    } else {
      Serial.print("Erro:");
      Serial.print(client.state());
      Serial.println(" reconectando em 5 segundos");
    // Colocar para conectar pelo celular
    //const char* ssid = "Galaxy A13 0109";
    //const char* pass = "njji1312";
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

  server.handleClient();  //
  sensors.requestTemperatures();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  long now = millis();

  if (now - lastTemp > 500) {
    lastTemp = now;
    for (int i = 0; i < numberOfDevices; i++) {
      // Search the wire for address
      if (sensors.getAddress(tempDeviceAddress, i)) {
        // Output the device ID
        // Serial.print("Temperature for device: ");
        // Serial.println(i,DEC);
        // Print the data
        temp[i] = sensors.getTempC(tempDeviceAddress);
        // float tempC = sensors.getTempC(tempDeviceAddress);
        // Serial.print("Temp C: ");
        // Serial.print(tempC);
        // Serial.print(" Temp F: ");
      }
    }
    char tempTopstr[8];
    char tempCenterstr[8];
    char tempBottomstr[8];
    Serial.println("Temperatura topo: ");
    Serial.println(temp[0]);
    Serial.println("ºC");
    Serial.println("Temperatura centro: ");
    Serial.println(temp[1]);
    Serial.println("ºC");
    Serial.println("Temperatura base: ");
    Serial.println(temp[2]);
    Serial.println("ºC");
    dtostrf(temp[0], 1, 1, tempTopstr);
    dtostrf(temp[1], 1, 1, tempCenterstr);
    dtostrf(temp[2], 1, 1, tempBottomstr);
    client.publish("ambar/CEV/temp_reatorB/top", tempTopstr);
    client.publish("ambar/CEV/temp_reatorB/center", tempCenterstr);
    client.publish("ambar/CEV/temp_reatorB/bottom", tempBottomstr);
    // ***********************************************************************************************************
    // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
    //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
    //
    //              ("           T    E    M    P    E    R    A    T    U    R    A:               ");
    //              ("      X    X    ,    X    |    X    X    ,    X    |    X    X     ,   X      ");
    // ***********************************************************************************************************
    lcd.setCursor(1, 4);  // Coluna, Linha
    lcd.print(tempTopstr);
    lcd.setCursor(6, 4);  // Coluna, Linha
    lcd.print(tempCenterstr);
    lcd.setCursor(11, 4);  // Coluna, Linha
    lcd.print(tempBottomstr);
  }
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
