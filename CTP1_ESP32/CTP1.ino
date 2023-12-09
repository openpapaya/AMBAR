// ESP32 DEV KIT - AMBAR - Cliente Tanques Principais
#include <stdio.h>
#include <string.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Wire.h>

// iP for OTA: 192.168.1.6/

// WiFi
const char* host = "AMBAR-CTP1";
const char* SSID = "Guilherme OiFibra 2G";
const char* PASS = "zezewifi2805";
//const char* SSID ="Galaxy A13 0109";
//const char* PASS ="njji1312";
//const char* SSID ="UNB Wireless";
//const char* PASS ="";
const char* mqtt_server = "broker.hivemq.com";  // MQTT server eh o mesmo que "broker"? Sim

#define DHTTYPE DHT22
#define I2C_SLAVE_ADDRESS 0x42  // Endereco I2C para o ESP32

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
static bool hasIoTHub = false;
static const char* connectionString = "";
long lastTemp = 0;
volatile uint8_t flag_msgFromPayload = 0;
volatile uint8_t resultStrCmp = 1;
String receivedString;

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
  "<center><font size=4><b>AMBAR - CTP 1 - identifique-se</b></font></center>"
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

  Serial.begin(115200);                         // Serial communication
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);  // pino 16 (RX) e 17(TX) do ESP e pino P3.3(TX) e P3.4(RX) do MSP430
  setup_wifi();
  setup_server();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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

  if (topicStr == "ambar/preparador1/statusPrep") {
    if (payload[0] == 'G')  //
    {
    }
  }
}

void loop() {

  server.handleClient();  //
  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  //if (millis() - lastTemp > 500) {                      // A função millis() retorna um número indicando há quantos milissegundos o Arduino está ligado.
  //lastTemp = millis();
  if (Serial2.available()) {
    //Serial.print("receivedString: ");
    char bytesRead = Serial2.read();
    //receivedString[bytesRead] = '\0';  // Add a null terminator to the end of the string
    receivedString += bytesRead;
    if (bytesRead == 'A') {  // Nivel Tanque 1
      char nivelT1[3];
      //receivedString.toCharArray(nivelT1,3);
      nivelT1[0] = receivedString[0];
      nivelT1[1] = receivedString[1];
      nivelT1[2] = receivedString[2];
      client.publish("ambar/tanque1/nivel", nivelT1);
      Serial.print("nivel T1: ");
      Serial.print(nivelT1);
      Serial.print("\r\n");
      receivedString = "";  // Limpa a string
    }
    if (bytesRead == 'B') {  // Nivel tanque 2
      char nivelT2[3];
      //receivedString.toCharArray(nivelT1,3);
      nivelT2[0] = receivedString[0];
      nivelT2[1] = receivedString[1];
      nivelT2[2] = receivedString[2];
      client.publish("ambar/tanque2/nivel", nivelT2);
      Serial.print("nivel T2: ");
      Serial.print(nivelT2);
      Serial.print("\r\n");
      receivedString = "";  // Limpa a string
    }
    if (bytesRead == 'T') {  // Nivel tanque 2
      char tempT1[4];
      //receivedString.toCharArray(nivelT1,3);
      tempT1[0] = receivedString[0];
      tempT1[1] = receivedString[1];
      tempT1[2] = '.';                     // "," ASCII 
      tempT1[3] = receivedString[2];
      client.publish("ambar/tanque1/temp", tempT1);
      Serial.print("temperatura no tanque A: ");
      Serial.print(tempT1);
      Serial.print("\r\n");
      receivedString = "";  // Limpa a string
    }
    if (bytesRead == ';') {
      if (receivedString == "10;")  // --> Tanque 1
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "0");
        client.publish("ambar/tanque1/nodered", "Iniciando");
        client.publish("ambar/tanque1/statusPrep", "");
      } else if (receivedString == "11;")  // --> T1 ENCHENDO
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "1");
        client.publish("ambar/tanque1/nodered", "Enchendo");
        client.publish("ambar/tanque1/statusPrep", "");
      } else if (receivedString == "12;")  // "2" eh o codigo de msg de tanque com timer de evaporacao
      {
        //
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "2");
        client.publish("ambar/tanque1/nodered", "Evaporando cloro");
        client.publish("ambar/tanque1/statusPrep", "");
      } else if (receivedString == "13;")  // Evap OK/Pronto para receber nutrientes
      {
        // Tanque 1 pronto para receber nutrientes
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "3");  // "3" sera interpretado como pronto para receber nutrientes
        client.publish("ambar/tanque1/nodered", "Pronto");
      } else if (receivedString == "14;")  // "4" eh o codigo de msg de tanque em modo de irrigacao
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "4");  //
        client.publish("ambar/tanque1/nodered", "Regando");
      } else if (receivedString == "15;")  // "5" eh o codigo de msg de tanque vazio
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "5");  //
        client.publish("ambar/tanque1/nodered", "Vazio");
      } else if (receivedString == "16;")  // "6" eh o codigo de msg de problema no tanque
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque1/statusT1", "6");  //
        client.publish("ambar/tanque1/nodered", "Erro");
      }
      //***** Codificacao para o tanque 2 *****
      else if (receivedString == "20;")  // = Iniciando
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "0");
        client.publish("ambar/tanque2/nodered", "Iniciando");
        client.publish("ambar/tanque2/statusPrep", " ");
      } else if (receivedString == "21;")  // = 7 --> T2 enchendo
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "1");
        client.publish("ambar/tanque2/nodered", "Enchendo");
        client.publish("ambar/tanque2/statusPrep", " ");
      } else if (receivedString == "22;")  // "2" eh o codigo de msg de tanque com timer de evaporacao
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "2");
        client.publish("ambar/tanque2/nodered", "Evaporando cloro");
        client.publish("ambar/tanque2/statusPrep", " ");
      } else if (receivedString == "23;")  // T2: Evap OK/Pronto para receber nutrientes
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "3");  // "3" sera interpretado como pronto para receber nutrientes, "T2 informa ao MSP de que pode enviar nutrientes"
        client.publish("ambar/tanque2/nodered", "Pronto");
      } else if (receivedString == "24;")  // "A" eh o codigo de msg de tanque em modo de irrigacao
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "4");  //
        client.publish("ambar/tanque2/nodered", "Regando");
      } else if (receivedString == "25;")  // "B" eh o codigo de msg de tanque vazio
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "5");  //
        client.publish("ambar/tanque2/nodered", "Vazio");
      } else if (receivedString == "26;")  // "C" eh o codigo de msg de problema no tanque
      {
        Serial.print("MSPtoESP32: ");
        Serial.print(receivedString[0]);
        Serial.print(receivedString[1]);
        Serial.print("\r\n");
        client.publish("ambar/tanque2/statusT2", "6");  //
        client.publish("ambar/tanque2/nodered", "Erro");
      }
      receivedString = "";  // Limpa a string
    }
  }
  //}
}