/*
   Funcao: Conexao a rede GSM/LTE e obtencao de data e hora e exibicao no monitor serial
   Autor: Paulo Cesar Menegon de Castro
   Data de criacao: 20.02.2024
   Data de modificacao: 04.08.2024
   Biblioteca:  https://github.com/vshymanskyy/TinyGSM
   Referencias: https://www.arduino.cc/reference/en/libraries/tinygsm/
                https://github.com/vshymanskyy/TinyGSM
                https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G/blob/master/examples/Arduino_TinyGSM/AllFunctions/AllFunctions.ino
  */

// Definicao de constantes
#define TINY_GSM_MODEM_SIM7000
#define TINY_GSM_RX_BUFFER 1024  // Buffer RX com 1 Kb
#define SerialAT Serial1         // Serial do MODEM
#define GSM_PIN "3636"
#define UART_BAUD 115200
#define PIN_DTR 25
#define PIN_TX 27
#define PIN_RX 26
#define PWR_PIN 4

// Credenciais GPRS (usando chip M2M)
const char apn[] = "claro.com.br";  // APN
const char gprsUser[] = "claro";    // Usuario
const char gprsPass[] = "claro";    // Senha

// Bibliotecas
#include <TinyGsmClient.h>

// Cria objetos
TinyGsm modem(SerialAT);      // Cria objeto modem
TinyGsmClient client(modem);  // Cliente TinyGSM para conexao internet

// Estrutura de dados
typedef struct
{
  int year, month, day;
  int hour, minute, second;
  float gmt;
} GSM;
GSM gsmInfo;

void gsmConn() {
  // O restart() leva algum tempo, mas elimina possiveis conexoes persistentes.
  // Para inicio mais rapido utilize init()
  Serial.println("*********************************************************************************************************");
  Serial.println("Inicializando modem...");
  if (!modem.restart()) {
    Serial.println("Falha na inicializacao do modem!");
  } else {
    Serial.println("Modem inicializado com sucesso!");
    Serial.println("Nome Modem: " + modem.getModemName());
    Serial.println("Info Modem: " + modem.getModemInfo());
    Serial.println("CCID: " + modem.getSimCCID());
    Serial.println("IMEI: " + modem.getIMEI());
    Serial.println("IMSI: " + modem.getIMSI());
    Serial.println("Operadora: " + modem.getOperator());
    Serial.println("Qualidade sinal: " + String(modem.getSignalQuality()));
    Serial.println("*********************************************************************************************************");
    /* Define tipo de rede
      2 - Automatico
      13 - Somente GSM
      38 - Somente LTE
      51 - Somente GSM e LTE
    */
    modem.setNetworkMode(2);
    // Conexao APN
    apnConn();  // Conexao APN
  }
}

void apnConn() {
  Serial.println("Conectando a APN...");
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println("Falha na Conexao a APN!");
    Serial.println("*********************************************************************************************************");
  } else {
    Serial.println("Conectado a APN!");
    IPAddress local = modem.localIP();
    Serial.println("IP Local: " + local);
    Serial.println("*********************************************************************************************************");
  }
}

void infoGSM() {  // Obtem informacoes da rede celular
  modem.getNetworkTime(&gsmInfo.year, &gsmInfo.month, &gsmInfo.day, &gsmInfo.hour, &gsmInfo.minute, &gsmInfo.second, &gsmInfo.gmt);
}

void setup() {
  // Inicia serial
  Serial.begin(115200);
  // Liga o MODEM
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(300);
  digitalWrite(PWR_PIN, LOW);
  // Inicializa o MODEM
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);
  gsmConn();
  // Destrava o SIM Card se necessario
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }
}

void loop() {
  // Verifica se esta conectado ao WiFi
  if (modem.isGprsConnected()) {
    Serial.println("Conectado a APN!");
    infoGSM();
    Serial.print("Data GSM: ");
    Serial.print(gsmInfo.day);
    Serial.print("/");
    Serial.print(gsmInfo.month);
    Serial.print("/");
    Serial.print(gsmInfo.year);
    Serial.println();
    Serial.print("Hora GSM: ");
    Serial.print(gsmInfo.hour);
    Serial.print(":");
    Serial.print(gsmInfo.minute);
    Serial.print(":");
    Serial.print(gsmInfo.second);
    Serial.println();
    Serial.println();
  } else {
    Serial.println("Tentando restabelecer conexao com a APN...");
    Serial.println("*********************************************************************************************************");
    gsmConn();
  }
  delay(1000);
}