/*
 * Funcao: Conexao ao WiFi e NTP para obter data e hora a cada segundo e exibir no monitor serial
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Biblioteca:  https://github.com/taranais/NTPClient
 * Referencias: https://randomnerdtutorials.com/esp32-ntp-client-date-time-arduino-ide/#more-66810
 */

// Bibliotecas
#include <WiFi.h>       // Biblioteca WiFi
#include <NTPClient.h>  // Servico de horario e data via servidor de rede

// Estrutura de dados
typedef struct  // Estrutura de dados para data e hora NTP
{
  int year, month, day;
  int hour, minute, second;
} NTP;
NTP ntpInfo;

// Cria objetos
WiFiClient clienteWiFi;   // Objeto WiFi
WiFiUDP udp;    // Acesso a UDP
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);  // Acesso ao NTP (udp, servidor, fuso hourrio, hour em seconds)
int count = 0;    // Tempo decorrido para tentar conexao ao WiFi

// Credenciais da rede
const char* ssid = "ssid";    // Nome da rede
const char* wifipass = "password";    // Senha

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
  ntp.forceUpdate();                                // Força o update das informacoes do NTP
  String data = ntp.getFormattedDate();             // Armazena obtida do NTP
  ntpInfo.year = (data.substring(0, 4)).toInt();     // Data do NTP
  ntpInfo.month = (data.substring(5, 7)).toInt();
  ntpInfo.day = (data.substring(8, 10)).toInt();
  ntpInfo.hour = ntp.getHours();  // hora do NTP
  ntpInfo.minute = ntp.getMinutes();
  ntpInfo.second = ntp.getSeconds();
}

void setup() {
  Serial.begin(115200);  // Inicia conexao serial
  // Seta WiFi para modo estacao e desconecta do modo AP se previamente conectado
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Inicia conexao ao WiFi
  wifiConn();
}

void loop() {
  // Verifica se esta conectado ao WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao WiFi!");
    infoNTP();
    Serial.print("Data NTP: ");
    Serial.print(ntpInfo.day);
    Serial.print("/");
    Serial.print(ntpInfo.month);
    Serial.print("/");
    Serial.print(ntpInfo.year);
    Serial.println();
    Serial.print("hour NTP: ");
    Serial.print(ntpInfo.hour);
    Serial.print(":");
    Serial.print(ntpInfo.minute);
    Serial.print(":");
    Serial.print(ntpInfo.second);
    Serial.println();
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