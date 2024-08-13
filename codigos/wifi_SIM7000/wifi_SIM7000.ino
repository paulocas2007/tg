/*
 * Funcao: Conexao basica a rede WiFi
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Biblioteca:  https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/src/WiFi.h
 * Referencias: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino
*/

// Bibliotecas
#include <WiFi.h>  // Biblioteca WiFi

// Credenciais da rede WiFi
const char* ssid = "ssid";          // Nome da rede
const char* wifipass = "password";  // Senha
int count = 0;                      // Tempo decorrido para tentar conexao ao WiFi

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
      if (count == 19) {  //Tenta conexao ao WiFi por 10 segundos
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

void setup() {
  Serial.begin(115200);  // Inicia conexao serial
  // Seta WiFi para modo estacao e desconecta do modo AP se previamente conectado
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Inicia conexao ao WiFi
  wifiConn();
}

void loop() {
}