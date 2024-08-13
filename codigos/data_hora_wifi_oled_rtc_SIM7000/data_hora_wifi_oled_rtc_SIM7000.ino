/*
 * Funcao: Exibicao de data e hora NTP no display OLED
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 04.08.2024
 * Bibliotecas:  https://github.com/taranais/NTPClient
 *               https://github.com/adafruit/Adafruit_SSD1306
 *               https://github.com/adafruit/Adafruit-GFX-Library
 * Referencias:  https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/#more-66810
 *               https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
 */

// Definicao de constantes
#define SCREEN_WIDTH 128  // Largura do display em pixels
#define SCREEN_HEIGHT 64  // Altura do display em pixels

// Bibliotecas
#include <WiFi.h>              // Biblioteca WiFi
#include <NTPClient.h>         // Servico de horario via servidor de rede
#include <Wire.h>              // Comunicacao I2C
#include <Adafruit_GFX.h>      // Controle do display OLED
#include <Adafruit_SSD1306.h>  // Controle do display OLED
#include <ESP32Time.h>         // Real Time Clock do ESP32

// Estrutura de dados
typedef struct  // Estrutura de dados para data e hora NTP
{
  int year, month, day;
  int hour, minute, second;
} NTP;
NTP ntpInfo;
typedef struct  // Estrutura para dados do RTC
{
  int year, month, day;
  int hour, minute, second;
  int milis;
} TIME;
TIME timeInfo;

// Cria objetos
WiFiClient clienteWiFi;                                            // Objeto WiFi
WiFiUDP udp;                                                       // Acesso a UDP
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);              // Acesso ao NTP (udp, servidor, fuso hourrio, hour em seconds)
ESP32Time rtc;                                                     // RTC do ESP32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // Display SSD1306 display conectado ao I2C (SDA, SCL pins)

// Credenciais da rede
const char* ssid = "ssid";          // Nome da rede
const char* wifipass = "password";  // Senha

int count = 0;  // Tempo decorrido para tentar conexao ao WiFi

void wifiConn() {
  bool wifi = false;  // Retorna true se a rede com o ssid desejado for encontrada
  Serial.println("*********************************************************************************************************");
  Serial.println("Buscando por redes WiFi...");
  // Procura redes WiFi e retorna numero de redes encontradas
  int net = WiFi.scanNetworks();
  Serial.println("Busca encerrada");
  if (net == 0) {
    Serial.println("Nenhuma rede encontrada!");
  } else {
    Serial.print(net);
    Serial.println(" redes encontradas:");
    for (int i = 0; i < net; ++i) {
      // Mostra SSID e RSSI de cada rede encontrada
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println();
      if (WiFi.SSID(i) == ssid)
        wifi = true;
      delay(10);
    }
  }
  // Inicia conexão com a rede WiFi se a rede ssid for encontrada
  if (wifi) {
    int count = 0;
    WiFi.begin(ssid, wifipass);
    Serial.println("*********************************************************************************************************");
    Serial.println("Estabelecendo conexao com a rede WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      count++;
      if (count == 19) {  //Tenta conexao ao WiFi por 10 seconds
        Serial.println();
        Serial.println("Não foi possível fazer a conexão com o WiFi!");
        Serial.println("*********************************************************************************************************");
        count = 0;
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Conexão bem sucedida!!!");
      Serial.print("Endereço IP: ");
      Serial.println(WiFi.localIP());
      long rssi = WiFi.RSSI();
      Serial.print("RSSI:");
      Serial.println(rssi);
      Serial.print("Status da conexão: ");
      Serial.println(WiFi.status());
      WiFi.printDiag(Serial);
      Serial.println("*********************************************************************************************************");
    }
  } else {
    Serial.println("A rede SSID nao foi encontrada!");
    Serial.println("*********************************************************************************************************");
  }
}

// Obtem data e hour via NTP usando rede WiFi
void infoNTP() {
  ntp.forceUpdate();                              // Força o update das informacoes do NTP
  String data = ntp.getFormattedDate();           // Armazena obtida do NTP
  ntpInfo.year = (data.substring(0, 4)).toInt();  // Data do NTP
  ntpInfo.month = (data.substring(5, 7)).toInt();
  ntpInfo.day = (data.substring(8, 10)).toInt();
  ntpInfo.hour = ntp.getHours();  // hora do NTP
  ntpInfo.minute = ntp.getMinutes();
  ntpInfo.second = ntp.getSeconds();
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
  Serial.begin(115200);  // Inicia conexao serial
  // Seta WiFi para modo estacao e desconecta do modo AP se previamente conectado
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Inicia conexao ao WiFi
  wifiConn();
  if (WiFi.status() == WL_CONNECTED) {
    // Ajuste do RTC (segundo, minuto, hora, dia, mes, ano)
    infoNTP();
    rtc.setTime(ntpInfo.second, ntpInfo.minute, ntpInfo.hour, ntpInfo.day, ntpInfo.month, ntpInfo.year);
    Serial.println("RTC atualizado!");
    Serial.println("*********************************************************************************************************");
  } else {
    Serial.println("Nao foi possivel atualizar o RTC!");
    Serial.println("*********************************************************************************************************");
  }
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
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao WiFi!");
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
    Serial.println("*********************************************************************************************************");
    Serial.println("Tentando restabelecer conexao com a rede WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      delay(500);
      Serial.print(".");
      count++;
      if (count == 19) {  //Tenta conexao ao WiFi por 10 seconds
        Serial.println();
        Serial.println("Não foi possível fazer a conexão com o WiFi!");
        count = 0;
        break;
      }
    }
  }
  delay(1000);
}