/*
  Funcao: Datalogger para armazenar dados do MPU6050 no SD Card utilizando WiFi e HTTP POST
  Autor: Paulo Cesar Menegon de Castro
  Criado em: 10.05.2024
  Modificado em: 06.07.2024
  Referencias: https://github.com/Xinyuan-LilyGO/LilyGO-T-SIM7000G/blob/master/examples/Arduino_TinyGSM/AllFunctions/AllFunctions.ino
               http://www.cplusplus.com/reference/ctime/strftime/

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

// Definicao de constantes
#define SD_MISO 2  // Pinos do SD Card
#define SD_MOSI 15
#define SD_SCLK 14
#define SD_CS 13
#define VERDE 32          // Botao para aquisicao dos dados
#define BRANCO 33         // Botao para gravar os dados no bd
#define PRETO 25          // Botao para apagar os dados no SD
#define LED 12            // Led da placa
#define SCREEN_WIDTH 128  // Largura do display em pixels
#define SCREEN_HEIGHT 64  // Altura do display em pixels

// Bibliotecas
#include <WiFi.h>              // Biblioteca WiFi
#include <ESP_SSLClient.h>     // Cliente SSL para HTTPS
#include <NTPClient.h>         // Servico de horario via servidor de rede
#include <ESP32Time.h>         // Real Time Clock do ESP32
#include <Adafruit_MPU6050.h>  // MPU6050
#include <Adafruit_Sensor.h>
#include <Wire.h>          // Comunicacao I2C
#include <SD.h>            // SD Card
#include <FS.h>            // Sistema de arquivos do SD
#include <SPI.h>           // Comunicacao SPI
#include <Adafruit_GFX.h>  // Display OLED
#include <Adafruit_SSD1306.h>

// Instancia timer
hw_timer_t* timer0 = NULL;
hw_timer_t* timer1 = NULL;

// Criacao de objetos
WiFiClient clienteWiFi;                                            // WiFi
ESP_SSLClient clienteSSL;                                          // SSL
WiFiUDP udp;                                                       // UDP
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);              // Acesso ao NTP (udp, servidor, fuso horario, hora em segundos)
ESP32Time rtc;                                                     // RTC do ESP32
Adafruit_MPU6050 mpu;                                              // MPU6050
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);  // Display OLED

// Credenciais da rede WiFi
//char* ssid = "10STUDIOS_DECO_VV";
//char* senhaWiFi = "IGUATINGA1010LR";
char* ssid = "Anna Cecilia 2013";
char* senhaWiFi = "18052013";
// Credenciais do servidor
const char servidor[] = "ideanet.com.br";           // Nome do dominio
const char recurso[] = "/insert.php";               // Recurso utilizado do servidor
const int porta = 443;                              // Numero da porta do servidor (HTTPS)

// Variaveis
typedef struct  // Estrutura de dados NTP
{
  int ano, mes, dia;
  int hora, minuto, segundo;
} NTP;
NTP ntpInfo;
typedef struct  // Estrutura para dados do RTC
{
  int ano, mes, dia;
  int hora, minuto, segundo;
  int milis;
} TIME;
TIME timeInfo;
typedef struct  // Estrutura de dados do MPU
{
  float gx, gy, gz;  // Eixos x, y e z do giroscopio
  float ax, ay, az;  // Eixos x, y e z do acelerometro
} MPU;
MPU mpuData;
typedef struct {  // Estrutura de dados dos botoes
  int GPIO;       // GPIO no qual o botao esta conectado
  bool press;     // Botao pressionado
} BOTAO;
BOTAO verde = { 32, false };
BOTAO branco = { 33, false };
BOTAO preto = { 25, false };
long int leituras = 0;         // Leituras do MPU6050
int freq_amostra = 20;         // Frequencia de amostragem em Hz
bool cartao = false;           // Cartao montado corretamente
String sensor = "MPU6050";     // Nome do sensor
unsigned long tempoVerde = 0;  // Variaveis para debouncing dos botoes
unsigned long ultimoVerde = 0;
unsigned long tempoBranco = 0;
unsigned long ultimoBranco = 0;
unsigned long tempoPreto = 0;
unsigned long ultimoPreto = 0;
bool intTimer0 = false;  // Flag de interrupcaodo Timer0
bool intTimer1 = false;  // Flag de interrupcaodo Timer1
bool estVerde = false;   // Estado do botao verde

// Interrupcao do Timer0
void IRAM_ATTR rotTimer0() {  // Armazena na RAM que e mais rapida que a FLASH
  intTimer0 = true;
}

// Interrupcao do Timer1
void IRAM_ATTR rotTimer1() {  // Armazena na RAM que e mais rapida que a FLASH
  intTimer1 = true;
}

// Inicializacao do timer
void startTimer() {
  timer0 = timerBegin(0, 80, true);
  timer1 = timerBegin(1, 80, true);
  /* Inicialização do timer. Parametros :
     0 - seleção do timer a ser usado, de 0 a 3.
     80 - prescaler. O clock principal do ESP32 é 80MHz. Dividimos por 80 para ter 1us por tick.
     true - true para contador progressivo, false para regressivo
  */
  timerAttachInterrupt(timer0, &rotTimer0, true);
  timerAttachInterrupt(timer1, &rotTimer1, true);
  /* Conecta à interrupção do timer:
     - timer é a instância do hw_timer
     - endereço da função a ser chamada pelo timer
     - edge=true gera uma interrupção
  */
  timerAlarmWrite(timer0, 1000000 / freq_amostra, true);
  timerAlarmWrite(timer1, 100000, true);
  /* - o timer instanciado no inicio
       - o valor em us
       - auto-reload. true para repetir o alarme de interrupcao
  */
  //Ativa o alarme
  timerAlarmEnable(timer0);
  timerAlarmEnable(timer1);
}

// Interrupcao botao verde
void IRAM_ATTR isrVerde() {
  tempoVerde = millis();
  if (tempoVerde - ultimoVerde > 300) {  // Compara o intervalo entre apertos dos botoes
    estVerde = !estVerde;                // para evitar bouncing
    ultimoVerde = tempoVerde;
  }
}

// Interrupcao do botao branco
void IRAM_ATTR isrBranco() {
  tempoBranco = millis();
  if (tempoBranco - ultimoBranco > 300) {
    branco.press = true;
    ultimoBranco = tempoBranco;
  }
}

// Interrupcao botao preto
void IRAM_ATTR isrPreto() {
  tempoPreto = millis();
  if (tempoPreto - ultimoPreto > 300) {
    preto.press = true;
    ultimoPreto = tempoPreto;
  }
}

void conexaoWiFi() {
  bool rede = false;  // Retorna true se a rede com o ssid desejado for encontrada
  Serial.println("*****************************************************************");
  Serial.println("Buscando por redes WiFi...");
  // Procura redes WiFi e retorna numero de redes encontradas
  int n = WiFi.scanNetworks();  // Numero de redes WiFi encontradas
  // Conexao com a rede WiFi
  Serial.println("Busca encerrada");
  if (n == 0) {
    Serial.println("Nenhuma rede encontrada!");
  } else {
    Serial.print(n);
    Serial.println(" redes encontradas:");
    for (int i = 0; i < n; ++i) {
      // Mostra SSID e RSSI de cada rede encontrada
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println();
      if (WiFi.SSID(i) == ssid) {
        rede = true;
      }
    }
  }
  // Inicia conexão com a rede WiFi se a rede ssid for encontrada
  if (rede) {
    int cont = 0;
    WiFi.begin(ssid, senhaWiFi);
    Serial.println("*****************************************************************");
    Serial.println("Estabelecendo conexao com a rede WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      cont++;
      if (cont == 19) {  //Tenta conexao ao WiFi por 10 segundos
        Serial.println();
        Serial.println("Não foi possível fazer a conexão com o WiFi!");
        Serial.println("*****************************************************************");
        display.println("Nao conectado ao WiFi!");
        display.display();
        cont = 0;
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println();
      Serial.println("Conexao bem sucedida!!!");
      display.println("Conectado ao WiFi!");
      display.display();
      Serial.print("Endereço IP: ");
      Serial.println(WiFi.localIP());
      long rssi = WiFi.RSSI();
      Serial.print("RSSI:");
      Serial.println(rssi);
      Serial.print("Status da conexão: ");
      Serial.println(WiFi.status());
      WiFi.printDiag(Serial);
      Serial.println("*****************************************************************");
    }
  } else {
    Serial.println("A rede SSID nao foi encontrada!");
    Serial.println("*****************************************************************");
  }
}

// Obtem data e hora via NTP usando rede WiFi
void ntpDataHora() {
  ntp.forceUpdate();                             // Força o update das informacoes do NTP
  String data = ntp.getFormattedDate();          // Armazena obtida do NTP
  ntpInfo.ano = (data.substring(0, 4)).toInt();  // Data do NTP
  ntpInfo.mes = (data.substring(5, 7)).toInt();
  ntpInfo.dia = (data.substring(8, 10)).toInt();
  ntpInfo.hora = ntp.getHours();  // Hora do NTP
  ntpInfo.minuto = ntp.getMinutes();
  ntpInfo.segundo = ntp.getSeconds();
}

void timeDataHora() {
  timeInfo.dia = rtc.getDay();
  timeInfo.mes = rtc.getMonth() + 1;
  timeInfo.ano = rtc.getYear();
  timeInfo.hora = rtc.getHour(true);
  timeInfo.minuto = rtc.getMinute();
  timeInfo.segundo = rtc.getSecond();
  timeInfo.milis = rtc.getMillis();
}

void mpuAjuste() {
  Serial.print("Conectando MPU6050...");
  if (!mpu.begin(0x69)) {  // Inicializa o MPU6050 no endereco 0x69
    Serial.println("Sensor MPU6050 não encontrado!");
  } else {
    Serial.println("Sensor MPU6050 encontrado!");
    mpu.setAccelerometerRange(MPU6050_RANGE_2_G);  // Ajusta acelerometro para +/- 2g
    Serial.print("Acelerometro ajustado para: ");
    switch (mpu.getAccelerometerRange()) {
      case MPU6050_RANGE_2_G:
        Serial.println("+-2G");
        break;
      case MPU6050_RANGE_4_G:
        Serial.println("+-4G");
        break;
      case MPU6050_RANGE_8_G:
        Serial.println("+-8G");
        break;
      case MPU6050_RANGE_16_G:
        Serial.println("+-16G");
        break;
    }
    mpu.setGyroRange(MPU6050_RANGE_250_DEG);  // Ajusta giroscopio para 250 graus/segundo
    Serial.print("Giroscopio ajustado para: ");
    switch (mpu.getGyroRange()) {
      case MPU6050_RANGE_250_DEG:
        Serial.println("+- 250 deg/s");
        break;
      case MPU6050_RANGE_500_DEG:
        Serial.println("+- 500 deg/s");
        break;
      case MPU6050_RANGE_1000_DEG:
        Serial.println("+- 1000 deg/s");
        break;
      case MPU6050_RANGE_2000_DEG:
        Serial.println("+- 2000 deg/s");
        break;
    }
    mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);  // Ajuste do filtro de interferencias
    Serial.print("Filtro ajustado para: ");
    switch (mpu.getFilterBandwidth()) {
      case MPU6050_BAND_260_HZ:
        Serial.println("260 Hz");
        break;
      case MPU6050_BAND_184_HZ:
        Serial.println("184 Hz");
        break;
      case MPU6050_BAND_94_HZ:
        Serial.println("94 Hz");
        break;
      case MPU6050_BAND_44_HZ:
        Serial.println("44 Hz");
        break;
      case MPU6050_BAND_21_HZ:
        Serial.println("21 Hz");
        break;
      case MPU6050_BAND_10_HZ:
        Serial.println("10 Hz");
        break;
      case MPU6050_BAND_5_HZ:
        Serial.println("5 Hz");
        break;
    }
  }
}

// Leitura do MPU6050
void mpuDados() {
  sensors_event_t acel, giro, temp;  // Leituras dos dados do sensor
  mpu.getEvent(&acel, &giro, &temp);
  mpuData.ax = acel.acceleration.x;  // Valores do acelerometro
  mpuData.ay = acel.acceleration.y;
  mpuData.az = acel.acceleration.z;
  mpuData.gx = giro.gyro.x;  // Valores do giroscopio
  mpuData.gy = giro.gyro.y;
  mpuData.gz = giro.gyro.z;
}

void criaArq() {
  // Cria o arquivo para armazenar os dados
  File arquivo = SD.open("/dados.csv");
  Serial.println("Criando arquivo...");
  if (!arquivo) {
    Serial.println("*****************************************************************");
    escreveArq(SD, "/dados.csv", "data;hora;milis;ax;ay;az;gx;gy;gz;freq_amostra\r\n");
  } else {
    Serial.println("O arquivo já existe!");
  }
  arquivo.close();
}

void escreveArq(fs::FS& fs, String caminho, String conteudo) {
  Serial.println("Escrevendo arquivo...");
  File arquivo = fs.open(caminho, FILE_WRITE);
  if (!arquivo) {
    Serial.println("Falha ao abrir arquivo para a escrita!");
    Serial.println("*****************************************************************");
    return;
  }
  if (arquivo.print(conteudo)) {
    Serial.println("Escrita realizada no arquivo!");
    Serial.println("*****************************************************************");
  } else {
    Serial.println("Falha ao escrever no arquivo!");
    Serial.println("*****************************************************************");
  }
  arquivo.close();
}

void apagaArq(fs::FS& fs, String caminho) {
  if (cartao) {
    if (fs.remove(caminho)) {
      Serial.println("Arquivo apagado!");
      Serial.println("*****************************************************************");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Apagando arquivo");
      display.println("SD Card!");
      display.display();
      digitalWrite(LED, LOW);
      delay(1000);
      digitalWrite(LED, HIGH);
      criaArq();
    } else {
      Serial.println("Falha ao apagar arquivo!");
      Serial.println("*****************************************************************");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Falha ao apagar!");
      display.display();
    }
  } else {
    Serial.println("Cartao nao montado!");
    Serial.println("Falha ao apagar arquivo!");
    Serial.println("*****************************************************************");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Falha ao apagar!");
    display.display();
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED, LOW);
      delay(100);
      digitalWrite(LED, HIGH);
      delay(100);
    }
  }
}

void abreBD() {
  Serial.print("Conectando a ");
  Serial.println(servidor);
  if (clienteSSL.connect(servidor, porta))
  {
    Serial.println("Conexao ao servidor bem sucedida!");
  } else {
    Serial.println("Falha na conexao ao servidor!");
    Serial.println("*******************************************");
  }
}

void fechaBD() {
  Serial.println("Desconectando do servidor...");
  clienteSSL.flush();
  clienteSSL.stop();  // Fecha conexao com o servidor
  Serial.println("Servidor desconectado!");
  Serial.println("*******************************************");
}

void leArq(fs::FS& fs, String caminho) {
  char caractere;         // Caractere do arquivo lido do SD Card
  long int amostras = 0;  // Numero de amostras gravadas no SD
  int registro = 0;       // Numero do campo do registro lido
  String dado = "";
  String campo_bd[10];  // Campos do registro lido
  String sql = "";
  File arquivo = fs.open(caminho);
  if (!arquivo) {
    Serial.println("Falha ao abrir arquivo para leitura!");
    Serial.println("*****************************************************************");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Falha ao abrir");
    display.println(" arquivo!");
    display.display();
    for (int i = 0; i < 10; i++) {
      digitalWrite(LED, LOW);
      delay(100);
      digitalWrite(LED, HIGH);
      delay(100);
    }
  }
  abreBD();
  Serial.println("Lendo arquivo do SD Card...");
  Serial.println("*****************************************************************");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Lendo SD Card!");
  display.display();
  while (arquivo.available()) {
    digitalWrite(LED, LOW);
    caractere = arquivo.read();
    if (caractere != '\n') {        // Se o caracter lido nao for o de final de linha (CR)
      if (caractere != ';') {       // Se o caracter lido nao for o de separacao dos registros (CSV)
        dado.concat(caractere);     // Concactena os caracteres do registro
      } else {                      // Se encontrou o caracter de separacao de registros (;)
        campo_bd[registro] = dado;  // Campo com o dado com caracteres concactenados
        dado = "";
        registro++;
      }
    } else {  // Se encontrou o fim de linha (CR)
      campo_bd[registro] = dado;
      dado = "";
      registro = 0;
      if (campo_bd[0] != "data" && campo_bd[0] != "0000-00-00")  // Ignora o cabecalho do aqruivo
        amostras++;
      if (amostras > 0) {
        sql = "sensor=" + sensor
              + "&data=" + campo_bd[0]
              + "&hora=" + campo_bd[1]
              + "&milis=" + campo_bd[2]
              + "&ax=" + campo_bd[3] + "&ay=" + campo_bd[4] + "&az=" + campo_bd[5]
              + "&gx=" + campo_bd[6] + "&gy=" + campo_bd[7] + "&gz=" + campo_bd[8]
              + "&freq_amostra=" + campo_bd[9];
        Serial.println(sql);
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Gravando no BD!");
        display.print("Amostra: ");
        display.print(amostras);
        display.print("/");
        display.println(leituras);
        display.display();
        escreveBD(sql, amostras);
      }
    }
  }
  if (amostras == 0) {
    display.println("Arquivo vazio!");
    display.display();
  }
  amostras--;
  arquivo.close();
  fechaBD();
  digitalWrite(LED, HIGH);
  delay(2000);
}

void escreveBD(String sql, int amostras) {
  if (clienteSSL.connected()) {
    Serial.println("Realizando requisicao HTTPS POST...");
    Serial.print ("Amostra: ");
    Serial.println(amostras);
    clienteSSL.print("POST " + String(recurso) + " HTTP/1.1\r\n");
    clienteSSL.print("Host:" + String(servidor) + "\r\n");
    clienteSSL.print("Content-Type: application/x-www-form-urlencoded\r\n");
    clienteSSL.print("Content-Length: ");
    clienteSSL.print(sql.length());
    clienteSSL.print("\r\n\r\n");
    clienteSSL.print(sql);
    unsigned long ms = millis();
    while (!clienteSSL.available() && millis() - ms < 100)
    {
      delay(0);
    }
    while (clienteSSL.available())
    {
      Serial.print((char)clienteSSL.read());
    }
  }
  else {
    Serial.println("Falha na conexao ao servidor!");
    Serial.println("*******************************************");
  }
}

void oled() {
  // Atualiza data e hora
  timeDataHora();
  // Leitura do MPU
  mpuDados();
  /*
    // Atualiza data e hora
    Serial.print("Data timer: ");
    Serial.print(timeInfo.dia);
    Serial.print(".");
    Serial.print(timeInfo.mes);
    Serial.print(".");
    Serial.print(timeInfo.ano);
    Serial.println();
    Serial.print("Hora timer: ");
    Serial.print(timeInfo.hora);
    Serial.print(".");
    Serial.print(timeInfo.minuto);
    Serial.print(".");
    Serial.print(timeInfo.segundo);
    Serial.print(".");
    Serial.println(timeInfo.milis);
    Serial.println();
    Serial.println("Aceleracao: ");
    Serial.print("Eixo X = ");
    Serial.print(mpuData.ax);
    Serial.println(" m/s2");
    Serial.print("Eixo Y = ");
    Serial.print(mpuData.ay);
    Serial.println(" m/s2");
    Serial.print("Eixo Z = ");
    Serial.print(mpuData.az);
    Serial.println(" m/s2");
    Serial.println();
    Serial.println("Velocidade angular: ");
    Serial.print("Eixo X = ");
    Serial.print(mpuData.gx);
    Serial.println(" rad/s");
    Serial.print("Eixo Y = ");
    Serial.print(mpuData.gy);
    Serial.println(" rad/s");
    Serial.print("Eixo Z = ");
    Serial.print(mpuData.gz);
    Serial.println(" rad/s");
    Serial.println();
  */
  // Exibe dados no display OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Data: ");
  if (timeInfo.dia < 10) {
    display.print("0");
    display.print(timeInfo.dia);
  } else {
    display.print(timeInfo.dia);
  }
  display.print("/");
  if (timeInfo.mes < 10) {
    display.print("0");
    display.print(timeInfo.mes);
  } else {
    display.print(timeInfo.mes);
  }
  display.print("/");
  display.print(timeInfo.ano);
  display.println();
  display.print("Hora: ");
  if (timeInfo.hora < 10) {
    display.print("0");
    display.print(timeInfo.hora);
  } else {
    display.print(timeInfo.hora);
  }
  display.print(":");
  if (timeInfo.minuto < 10) {
    display.print("0");
    display.print(timeInfo.minuto);
  } else {
    display.print(timeInfo.minuto);
  }
  display.print(":");
  if (timeInfo.segundo < 10) {
    display.print("0");
    display.print(timeInfo.segundo);
  } else {
    display.print(timeInfo.segundo);
  }
  display.print(":");
  display.println(timeInfo.milis);
  display.println();
  display.println("X, Y, Z (m/s2): ");
  display.print(mpuData.ax);
  display.print(", ");
  display.print(mpuData.ay);
  display.print(", ");
  display.println(mpuData.az);
  display.println();
  display.println("X, Y, Z (rad/s): ");
  display.print(mpuData.gx);
  display.print(", ");
  display.print(mpuData.gy);
  display.print(", ");
  display.print(mpuData.gz);
  display.println();
  display.display();
}

void setup() {
  Serial.begin(115200);                              // Inicia serial
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Inicia display OLED
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  } else {
    display.clearDisplay();  // Limpa o display
    display.setCursor(0, 0);
    display.setTextSize(1);       // Tamanho do caracter
    display.setTextColor(WHITE);  // Cor do caracter
    display.println("Iniciando...");
    display.display();
  }
  pinMode(LED, OUTPUT);    // LED da placa
  digitalWrite(LED, LOW);  // Acende LED da placa
  WiFi.mode(WIFI_STA);     // Seta WiFi para modo estacao e desconecta do modo AP se previamente conectado
  WiFi.disconnect();
  // Inicia conexao ao WiFi
  conexaoWiFi();
  if (WiFi.status() == WL_CONNECTED) {
    ntpDataHora();  // Atualiza NTP
    // Ajuste do RTC (segundo, minuto, hora, dia, mes, ano)
    rtc.setTime(ntpInfo.segundo, ntpInfo.minuto, ntpInfo.hora, ntpInfo.dia, ntpInfo.mes, ntpInfo.ano);
    Serial.println("Timer da ESP32 atualizado!");
    Serial.println("*****************************************************************");
    display.println("Relogio atualizado!");
    display.display();
  } else {
    display.println("Falha conexao WiFi!");
    display.println("Relogio nao atualizado!");
    display.display();
  }
  clienteSSL.setInsecure();     // Ignora verificacao do SSL pelo servidor
  clienteSSL.setBufferSizes(1024, 1024);   // Tamanho do buffer RX e TX (512 to 16384).
  /** Nivel do debug
     esp_ssl_debug_none = 0
     esp_ssl_debug_error = 1
     esp_ssl_debug_warn = 2
     esp_ssl_debug_info = 3
     esp_ssl_debug_dump = 4
  */
  clienteSSL.setDebugLevel(1);
  //clienteSSL.setSessionTimeout(120);    // Encerra sessao ativa apos 120 segundos
  clienteSSL.setClient(&clienteWiFi);   // Cria camada SSL no cliente
  mpuAjuste();  // Ajuste do MPU
  Serial.println("*****************************************************************");
  Serial.println("Inicializando o SD Card...");
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS)) {
    Serial.println("Falha na montagem do SD Card!");
    display.println("Falha no SD Card!");
    display.display();
  } else {
    cartao = true;
    uint32_t capacidade = SD.cardSize() / (1024 * 1024);  // Capacidade do cartao em MB
    String str = "Capacidade do SD Card: " + String(capacidade) + "MB";
    display.println("Cartao SD OK!");
    display.display();
    Serial.println(str);
    Serial.println("*****************************************************************");
    // Cria o arquivo para armazenar os dados
    criaArq();
  }
  Serial.println("*****************************************************************");
  // Configuracoes dos pinos dos botoes
  pinMode(verde.GPIO, INPUT_PULLUP);
  attachInterrupt(verde.GPIO, isrVerde, FALLING);  // Botao ativo na borda de descida
  pinMode(branco.GPIO, INPUT_PULLUP);
  attachInterrupt(branco.GPIO, isrBranco, FALLING);
  pinMode(preto.GPIO, INPUT_PULLUP);
  attachInterrupt(preto.GPIO, isrPreto, FALLING);
  startTimer();             // Inicia o timer da ESP32
  digitalWrite(LED, HIGH);  // Apaga LED da placa
  delay(3000);
}

void loop() {
  // Atualiza display com dados do relogio e MPU
  if (intTimer1 && !estVerde) {  // Se interrupcao do Timer 1 e botao verde nao acionado
    intTimer1 = false;
    oled();
  }
  // Tempo de obtencao de dados do MPU e de gravacao no SD
  //long int tempo_MPU, tempo_SD;
  // Periodo de amostragem em milisegundos
  int per_amostra = 1000 / freq_amostra;
  // Verifica se botao verde pressionado
  if (estVerde) {
    // Se cartao montado corretamente
    if (cartao) {
      digitalWrite(LED, LOW);
      // Se houve interrupcao do timer
      if (intTimer0 && (rtc.getMillis() % (per_amostra)) == 0) {
        intTimer0 = false;
        // Atualiza data e hora
        timeDataHora();
        // Leitura do MPU
        //tempo_MPU = millis();
        mpuDados();
        //tempo_MPU = millis() - tempo_MPU;
        //Serial.print("Tempo de leitura do MPU: ");
        //Serial.print(tempo_MPU);
        //Serial.println(" milisegundos");
        //Serial.println("*****************************************************************");
        // Escreve dados no cartão SD
        String conteudo = String(timeInfo.ano) + "-" + String(timeInfo.mes) + "-" + String(timeInfo.dia) + ";";
        conteudo = conteudo + String(timeInfo.hora) + ":" + String(timeInfo.minuto) + ":" + String(timeInfo.segundo) + ";" + String(timeInfo.milis) + ";";
        conteudo = conteudo + String(mpuData.ax) + ";" + String(mpuData.ay) + ";" + String(mpuData.az) + ";";
        conteudo = conteudo + String(mpuData.gx) + ";" + String(mpuData.gy) + ";" + String(mpuData.gz) + ";";
        conteudo = conteudo + String(freq_amostra) + "\r\n";
        Serial.println("*****************************************************************");
        Serial.println(conteudo);
        //tempo_SD = millis();
        File arquivo = SD.open("/dados.csv", FILE_APPEND);
        if (arquivo.print(conteudo)) {
          leituras++;
          Serial.println("Escrita realizada no arquivo!");
          Serial.println("*****************************************************************");
        } else {
          Serial.println("Falha ao escrever no arquivo!");
          Serial.println("*****************************************************************");
        }
        arquivo.close();
        //tempo_SD = millis() - tempo_SD;
        //Serial.print("Tempo de escrita no SD Card: ");
        //Serial.print(tempo_SD);
        //Serial.println(" milisegundos");
        //Serial.println("*****************************************************************");
        //Serial.print("Tempo total: ");
        //Serial.print(tempo_MPU + tempo_SD);
        //Serial.println(" milisegundos");
        //Serial.println("*****************************************************************");
      }
    } else {
      Serial.println("Cartao nao montado!");
      Serial.println("Falha ao gravar dados no SD!");
      Serial.println("*****************************************************************");
      for (int i = 0; i < 10; i++) {
        digitalWrite(LED, LOW);
        delay(100);
        digitalWrite(LED, HIGH);
        delay(100);
      }
      estVerde = false;
    }
  } else {  // Se nao estiver adquirindo dados
    // Se botao branco pressionado, envia o arquivo para o BD
    if (branco.press) {
      branco.press = false;
      if (WiFi.status() == WL_CONNECTED) {
        leArq(SD, "/dados.csv");
      } else {
        Serial.println("Falha na rede WiFi!");
        Serial.println("*****************************************************************");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("Falha na rede WiFi!");
        display.display();
        delay(2000);
      }
    }  // Se botao preto pressionado, apaga o arquivo no SD
    if (preto.press) {
      preto.press = false;
      apagaArq(SD, "/dados.csv");
    }
  }
  digitalWrite(LED, HIGH);
}
