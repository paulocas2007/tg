/*
 * Funcao: Conexao ao bd via rede WiFi usando a biblioteca HTTPClient
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Biblioteca:  https://github.com/espressif/arduino-esp32/blob/master/libraries/HTTPClient/src/HTTPClient.h
 * Referencias: https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino

  CREATE TABLE sensor_mpu6050 (
  id INT(10) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    sensor VARCHAR (10),
    data DATE,
    hora TIME,
    milis INT (3),
    ax DECIMAL(4,2),
    ay DECIMAL(4,2),
    az DECIMAL(4,2),
    gx DECIMAL(4,2),
    gy DECIMAL(4,2),
    gz DECIMAL(4,2),
    freq_amostra INT(2)
  );
 */

// Bibliotecas
#include <WiFi.h>  // Biblioteca WiFi
#include <HTTPClient.h>   // Biblioteca HTTP

// Credenciais da rede WiFi
char* ssid = "10STUDIOS_DECO_VV";
char* wifipass = "IGUATINGA1010LR";
//const char* ssid = "ssid";  // Nome da rede
//const char* wifipass = "password";  // Senha
// Credenciais do servidor
const char servidor[] = "https://ideanet.com.br/insert.php";  
//const char recurso[] = "/insert.php";
//const int porta = 80;
int count = 0;    // Tempo decorrido para tentar conexao ao WiFi

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

void insertBD() {
  // Checa status da conexao
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setReuse(true);    // Permite reuso da conexao (keep alive)
    // Inicia cliente http
    http.begin(servidor);
    // Especifica o tipo de conteudo do cabecalho
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Prepara dados para o HTTP POST
    String sql = "sensor=MPU6050&data=2024-12-31&hora=00:00:00&milis=999&ax=10&ay=20&az=30&gx=40&gy=50&gz=60&freq_amostra=70";
    Serial.print("SQL: ");
    Serial.println(sql);
    // Envia requisicao HTTP POST
    int httpResponseCode = http.POST(sql);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Libera recursos
    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
}
void loop() {
  insertBD();
  delay (10000);
}
