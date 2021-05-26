/*
Necesario a instalação das bibliotecas mySQL, wifiNINA e max30100lib
arduino_secrets.h contém as informações sigilosas de ssdi e senha da wifi, bem como o login e a senha do banco de dados
*/

// INCLUSÃO DE BIBLIOTECAS
#include <SPI.h>
#include <WiFiNINA.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#include "arduino_secrets.h"

// DEFINIÇÕES
#define PIN_botao 7
#define REPORTING_PERIOD_MS  1000



// DECLARAÇÃO DE VARIÁVEIS PARA WIFI
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;        // your network password
int status = WL_IDLE_STATUS;      // status

// DECLARAÇÃO DE VARIÁVEIS PARA MySQL
IPAddress server_addr(85, 10, 205, 173);  // IP of the MySQL *server* here
char user[] = SECRET_USERDB;              // MySQL user login username
char password[] = SECRET_PASSDB;          // MySQL user login password

char INSERT_SQL[] = "INSERT INTO test.projbc (device_ID, tempo, batimentos, oxi) VALUES ('%d', '%d', '%f', '%f')";
char query[128];

// INSTANCIANDO OBJETOS
WiFiClient client;
MySQL_Connection conn((Client *)&client);

// DECLARAÇÃO DE FUNÇÕES
void conectaWifi();
void enviaDados();

// configuração logica do sensor

// PulseOximeter is the higher level interface to the sensor
// it offers:
//  * beat detection reporting
//  * heart rate calculation
//  * SpO2 (oxidation level) calculation
PulseOximeter pox;

uint32_t tsLastReport = 0;

// Callback (registered below) fired when a pulse is detected
void onBeatDetected()
{
    Serial.println("Beat!");
}

// ***************** INÍCIO DO SETUP *************************
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN_botao, INPUT_PULLUP);
  digitalWrite(LED_BUILTIN, LOW);
 
  while (!Serial) {
    ; // ESPERA O SERIAL ESTAR PRONTO
  }

  conectaWifi();

  // CONECTA NO MySQL
  while (!conn.connect(server_addr, 3306, user, password)) {
    Serial.println("Conexão SQL falhou.");
    conn.close();
    delay(1000);
    Serial.println("Conectando SQL novamente.");
  }
  Serial.println("Conectado ao servidor SQL.");
  digitalWrite(LED_BUILTIN, HIGH);
  
  Serial.print("Initializing pulse oximeter..");

    // Initialize the PulseOximeter instance
    // Failures are generally due to an improper I2C wiring, missing power supply
    // or wrong target chip
    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }
    // Register a callback for the beat detection
    pox.setOnBeatDetectedCallback(onBeatDetected);
}

// ***************** FIM DO SETUP ***************************

// ***************** INÍCIO DO LOOP *************************
void loop() {
// Make sure to call update as fast as possible
    pox.update();

    //verifica se o botão foi pressionado, se foi, então continua
    if ( digitalRead(PIN_botao) == LOW){
      // Asynchronously dump heart rate and oxidation levels to the serial
      // For both, a value of 0 means "invalid"
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        Serial.print("Heart rate:");
        Serial.print(pox.getHeartRate());
        Serial.print("bpm / SpO2:");
        Serial.print(pox.getSpO2());
        Serial.println("%");
        enviaDados(pox.getHeartRate(), pox.getSpO2());

        tsLastReport = millis();
      }
    }
}
// ***************** FIM DO LOOP ***************************

void conectaWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {                               //verifica o firmware do wifi
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
}

void enviaDados(float batimento, float oxi) {
  sprintf(query, INSERT_SQL, 1, millis()/1000, batimento, oxi);  //valores a serem eviados
  // Initiate the query class instance
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  // Execute the query
  cur_mem->execute(query);    //executa a string INSERT_SQL que adiciona as medições ao banco de dados
  delete cur_mem;     //libera memoria
  Serial.println("Informações Enviadas");
}
