/*
 * SD Card - Funcoes CRUD para arquivos de um SD Card - Create, Read, Update e Delete
 * Autor: Paulo Cesar Menegon de Castro
 * Criado em: 16.04.2024
 * Modificado em: 02.08.2024
 * Biblioteca:  https://github.com/espressif/arduino-esp32/blob/master/libraries/SD/README.md
 * Referencias: https://randomnerdtutorials.com/guide-to-sd-card-module-with-arduino/
 *              https://www.arduino.cc/reference/en/libraries/sd/
 *              https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
 *              https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G
 *
 * pin 1 - Nao usado         |  Micro SD card     |
 * pin 2 - CS (SS)           |                   /
 * pin 3 - DI (MOSI)         |                  |__
 * pin 4 - VDD (3.3V)        |                    |
 * pin 5 - SCK (SCLK)        | 8 7 6 5 4 3 2 1   /
 * pin 6 - VSS (GND)         | ▄ ▄ ▄ ▄ ▄ ▄ ▄ ▄  /
 * pin 7 - DO (MISO)         | ▀ ▀ █ ▀ █ ▀ ▀ ▀ |
 * pin 8 - Nao usado         |_________________|
 *                             ║ ║ ║ ║ ║ ║ ║ ║
 *                     ╔═══════╝ ║ ║ ║ ║ ║ ║ ╚═════════╗
 *                     ║         ║ ║ ║ ║ ║ ╚══════╗    ║
 *                     ║   ╔═════╝ ║ ║ ║ ╚═════╗  ║    ║
 * Conexoes para       ║   ║   ╔═══╩═║═║═══╗   ║  ║    ║
 * SD Card             ║   ║   ║   ╔═╝ ║   ║   ║  ║    ║
 * Tipo SD             ║   ║   ║   ║   ║   ║   ║  ║    ║
 * Funcao           |  -  DO  VSS SCK VDD VSS DI CS    -  |
 * SD pino          |  8   7   6   5   4   3   2   1   9 /
 *                  |                                  █/
 *                  |__▍__▊___█___█___█___█___█___█___/
 *
 * Nota:  Pinos SPI podem ser configurados manualmente via SPI.begin(SCK, MISO, MOSI, CS).`
 * 
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SPI Pin Name | ESP8266 | ESP32 | ESP32‑S2 | ESP32‑S3 | ESP32‑C3 | ESP32‑C6 | ESP32‑H2 |
 * +==============+=========+=======+==========+==========+==========+==========+==========+
 * | CS (SS)      | GPIO15  | GPIO5 | GPIO34   | GPIO10   | GPIO7    | GPIO18   | GPIO0    |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DI (MOSI)    | GPIO13  | GPIO23| GPIO35   | GPIO11   | GPIO6    | GPIO19   | GPIO25   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | DO (MISO)    | GPIO12  | GPIO19| GPIO37   | GPIO13   | GPIO5    | GPIO20   | GPIO11   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 * | SCK (SCLK)   | GPIO14  | GPIO18| GPIO36   | GPIO12   | GPIO4    | GPIO21   | GPIO10   |
 * +--------------+---------+-------+----------+----------+----------+----------+----------+
 *
 *  Formato do arquivo: CSV (separado por ;)
 */

// Pinos do SD Card
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13

// Bibliotecas
#include <SD.h>   // SD Card
#include <FS.h>   // Sistema de arquivos do SD
#include <SPI.h>  // Comunicacao SPI

// Variaveis
bool mountedSD = false;  // Cartao montado corretamente

void mountSD() {
  Serial.println("*********************************************************************************************************");
  Serial.println("Inicializando o SD Card...");
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS)) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
    Serial.println("*********************************************************************************************************");
  } else {
    Serial.println("Montagem do SD Card realizada com sucesso!");
    uint32_t capacidade = SD.cardSize() / (1024 * 1024);
    String str = "Capacidade do SD Card: " + String(capacidade) + "MB";
    Serial.println(str);
    mountedSD = true;
  }
}

void createFile(fs::FS& fs, String path) {
  // Verifica se cartao montado corretamente
  if (!mountedSD) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
    Serial.println("*********************************************************************************************************");
  } else {
    // Cria o arquivo para armazenar os datas
    File file = fs.open(path);
    Serial.println("Criando arquivo...");
    if (!file) {
      Serial.println("*********************************************************************************************************");
      writeFile(SD, "/datas.csv", "sensor;data;hora;milis;ax;ay;az;gx;gy;gz;freq_amostra\r\n");
    } else {
      Serial.println("O arquivo já existe!");
    }
    file.close();
  }
}

void writeFile(fs::FS& fs, String path, String text) {
  if (!mountedSD) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
    Serial.println("*********************************************************************************************************");
  } else {
    Serial.println("Escrevendo arquivo...");
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.println("Falha ao abrir arquivo para a escrita!");
      Serial.println("*********************************************************************************************************");
      return;
    }
    if (file.print(text)) {
      Serial.println("Escrita realizada no arquivo!");
      Serial.println("*********************************************************************************************************");
    } else {
      Serial.println("Falha ao escrever no arquivo!");
      Serial.println("*********************************************************************************************************");
    }
    file.close();
  }
}

void appendFile(fs::FS& fs, String path, String text) {
  if (!mountedSD) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
  } else {
    Serial.printf("Inserindo conteudo no arquivo: %s\n", path);
    File file = fs.open(path, FILE_APPEND);
    if (!file) {
      Serial.println("Falha ao abrir o arquivo para insercao!");
    }
    if (file.print(text)) {
      Serial.println("Conteudo inserido!");
      Serial.println("*********************************************************************************************************");
    } else {
      Serial.println("Falha ao inserir!");
      Serial.println("*********************************************************************************************************");
    }
    file.close();
  }
  Serial.println("*********************************************************************************************************");
}

void deleteFile(fs::FS& fs, String path) {
  if (!mountedSD) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
    Serial.println("*********************************************************************************************************");
  } else {
    if (fs.remove(path)) {
      Serial.println("Arquivo apagado!");
      Serial.println("*********************************************************************************************************");
    } else {
      Serial.println("Falha ao apagar arquivo!");
      Serial.println("*********************************************************************************************************");
    }
  }
}

void readFile(fs::FS& fs, String path) {
  char character;           // character do arquivo lido do SD Card
  long int recSample = 0;   // Numero de amostras gravadas no SD
  long int readSample = 0;  // Numero de amostras lidas para insercao no bd
  String data;              // characters concatenados lidos do SD Card
  String field_sd[11];      // fields do registro lido
  int field = 0;            // Numero do field do reg lido
  String reg;               // Registro para insercao no bd
  if (!mountedSD) {
    Serial.println("Falha na montagem do SD Card ou SD Card ausente!");
    Serial.println("*********************************************************************************************************");
  } else {
    File file = fs.open(path);
    if (!file) {
      Serial.println("Falha ao abrir arquivo para leitura!");
      Serial.println("*********************************************************************************************************");
    } else {
      Serial.println("Lendo arquivo do SD Card...");
      while (file.available()) {  // Quantidade de amostras no SD Card
        character = file.read();
        if (character == '\n')  // Se o caracter lido for o de final de linha (CR)
          recSample++;
      }
      file.close();
      recSample--;  // Desconsidera o cabecalho do arquivo
      Serial.print("Numero de amostras gravadas no SD Card: ");
      Serial.println(recSample);
      if (recSample > 0) {  // Leitura do reg encontrado
        File file = fs.open(path);
        if (!file) {
          Serial.println("Falha ao abrir arquivo para leitura!");
          Serial.println("*********************************************************************************************************");
        } else {
          Serial.println("Lendo arquivo do SD Card...");
          while (file.available()) {
            character = file.read();
            if (character != '\n') {     // Se o caracter lido nao for o de final de linha (CR)
              if (character != ';') {    // Se o caracter lido nao for o de separacao dos regs (CSV)
                data.concat(character);  // Concactena os characters do reg
              } else {                   // Se encontrou o caracter de separacao de regs (;)
                field_sd[field] = data;  // field com o data com characters concactenados
                data = "";
                field++;
              }
            } else {  // Se encontrou o fim de linha (CR)
              field_sd[field] = data;
              data = "";
              field = 0;
              readSample++;
              if (readSample > 1) {  // Ignora o cabecalho
                reg = "sensor=" + field_sd[0]
                      + "&data=" + field_sd[1]
                      + "&hora=" + field_sd[2]
                      + "&milis=" + field_sd[3]
                      + "&ax=" + field_sd[4] + "&ay=" + field_sd[5] + "&az=" + field_sd[6]
                      + "&gx=" + field_sd[7] + "&gy=" + field_sd[8] + "&gz=" + field_sd[9]
                      + "&freq_amostra=" + field_sd[10];
                Serial.println(reg);
              }
            }
          }
          readSample--;
          file.close();
          Serial.print("Numero de amostras lidas para insercao no bd: ");
          Serial.println(readSample);
          Serial.println("*********************************************************************************************************");
        }
      } else {
        Serial.println("Nenhuma amostra encontrada no SD Card: ");
      }
    }
  }
}

void setup(void) {
  Serial.begin(115200);  // Inicia comunicacao serial
  // data a ser inserido
  String data = String("MPU6050;2024-12-31;00:00:00;999;1;2;3;4;5;6;7\r\n");
  mountSD();
  createFile(SD, "/dados.csv");
  appendFile(SD, "/dados.csv", data);  // Write sobreescreve o arquivo
  readFile(SD, "/dados.csv");
  deleteFile(SD, "/dados.csv");
}

void loop() {
}
