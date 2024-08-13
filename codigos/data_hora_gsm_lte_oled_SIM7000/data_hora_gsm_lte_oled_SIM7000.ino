/*
 * Funcao: Exibicao de data e hora GSM no display OLED
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 04.08.2024
 * Bibliotecas:  https://github.com/vshymanskyy/TinyGSM
 *               https://github.com/adafruit/Adafruit_SSD1306
 *               https://github.com/adafruit/Adafruit-GFX-Library
 * Referencias:  https://www.arduino.cc/reference/en/libraries/tinygsm/
 *               https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
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
#define SCREEN_WIDTH 128  // Largura do display em pixels
#define SCREEN_HEIGHT 64  // Altura do display em pixels

// Credenciais GPRS (usando chip M2M)
const char apn[] = "claro.com.br";  // APN
const char gprsUser[] = "claro";    // Usuario
const char gprsPass[] = "claro";    // Senha

// Bibliotecas
#include <TinyGsmClient.h>
#include <Wire.h>              // Comunicacao I2C
#include <Adafruit_GFX.h>      // Controle do display OLED
#include <Adafruit_SSD1306.h>  // Controle do display OLED
#include <ESP32Time.h>         // Real Time Clock do ESP32

// Cria objetos
TinyGsm modem(SerialAT);                                           // Cria objeto modem
TinyGsmClient client(modem);                                       // Cliente TinyGSM para conexao internet
ESP32Time rtc;                                                     // RTC do ESP32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // Display SSD1306 display conectado ao I2C (SDA, SCL pins)

// Estrutura de dados
typedef struct
{
  int year, month, day;
  int hour, minute, second;
  float gmt;
} GSM;
GSM gsmInfo;
typedef struct  // Estrutura para dados do RTC
{
  int year, month, day;
  int hour, minute, second;
  int milis;
} TIME;
TIME timeInfo;

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

void infoTime() {
  timeInfo.day = rtc.getDay();
  timeInfo.month = rtc.getMonth() + 1;
  timeInfo.year = rtc.getYear();
  timeInfo.hour = rtc.getHour(true);
  timeInfo.minute = rtc.getMinute();
  timeInfo.second = rtc.getSecond();
  timeInfo.milis = rtc.getMillis();
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
  // Ajuste do RTC (segundo, minuto, hora, dia, mes, ano)
  infoGSM();
  rtc.setTime(gsmInfo.second, gsmInfo.minute, gsmInfo.hour, gsmInfo.day, gsmInfo.month, gsmInfo.year);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Endereco I2C do display = 0x3C
    Serial.println("Falha alocacao SSD1306");
    for (;;)
      ;
  }
  display.clearDisplay();       // Limpa o display
  display.setTextSize(1);       // Tamanho do caracter
  display.setTextColor(WHITE);  // Cor do caracter
  display.display();
}

void loop() {
  // Verifica se esta conectado ao WiFi
  if (modem.isGprsConnected()) {
    Serial.println("Conectado a APN!");
    infoTime();
    Serial.print("Data: ");
    Serial.print(timeInfo.day);
    Serial.print("/");
    Serial.print(timeInfo.month);
    Serial.print("/");
    Serial.print(timeInfo.year);
    Serial.println();
    Serial.print("Hora: ");
    Serial.print(timeInfo.hour);
    Serial.print(":");
    Serial.print(timeInfo.minute);
    Serial.print(":");
    Serial.print(timeInfo.second);
    Serial.println();
    Serial.println();
    // Exibe dados no display OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Data: ");
    if (timeInfo.day < 10) {
      display.print("0");
      display.print(timeInfo.day);
    } else {
      display.print(timeInfo.day);
    }
    display.print("/");
    if (timeInfo.month < 10) {
      display.print("0");
      display.print(timeInfo.month);
    } else {
      display.print(timeInfo.month);
    }
    display.print("/");
    display.print(timeInfo.year);
    display.println();
    display.println();
    display.println("Hora: ");
    if (timeInfo.hour < 10) {
      display.print("0");
      display.print(timeInfo.hour);
    } else {
      display.print(timeInfo.hour);
    }
    display.print(":");
    if (timeInfo.minute < 10) {
      display.print("0");
      display.print(timeInfo.minute);
    } else {
      display.print(timeInfo.minute);
    }
    display.print(":");
    if (timeInfo.second < 10) {
      display.print("0");
      display.print(timeInfo.second);
    } else {
      display.print(timeInfo.second);
    }
    display.println();
    display.display();
  } else {
    Serial.println("Tentando restabelecer conexao com a APN...");
    Serial.println("*********************************************************************************************************");
    gsmConn();
  }
  delay(1000);
}