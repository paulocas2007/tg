/*
 * Funcao: Utilizacao de funcoes de interrupcao e timer do ESP32
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Referencias: https://randomnerdtutorials.com/esp32-pir-motion-sensor-interrupts-timers/
 */

// Definicao de constantes
#define GREEN 32  // Botao verde
#define WHITE 33  // Botao branco
#define BLACK 25  // Botao preto

// Instancia timer
hw_timer_t* timer0 = NULL;
hw_timer_t* timer1 = NULL;

// Variaveis
typedef struct {  // Estrutura de dados dos botoes
  int GPIO;       // GPIO no qual o botao esta conectado
  bool press;     // Botao pressionado
} BOTTON;
// Struct dos botoes
BOTTON green = { 32, false };
BOTTON white = { 33, false };
BOTTON black = { 25, false };
// Debouncing dos botoes
unsigned long greenTime = 0;
unsigned long greenLast = 0;
unsigned long whiteTime = 0;
unsigned long whiteLast = 0;
unsigned long blackTime = 0;
unsigned long blackLast = 0;
bool intTimer0 = false;  // Flag de interrupcaodo Timer0
bool intTimer1 = false;  // Flag de interrupcaodo Timer1

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
  // Frequencia do timer em 1 MHz
  timer0 = timerBegin(1000000);  // Frequencia em Hz (maximo de 80MHz)
  timer1 = timerBegin(1000000);  // Frequencia em Hz (maximo de 80MHz)
  // Funcao a ser chamada pela interrupcao do timer.
  timerAttachInterrupt(timer0, &rotTimer0);
  timerAttachInterrupt(timer1, &rotTimer1);
  // Intervalo para chamada da funcao
  timerAlarm(timer0, 5000000, true, 0);   // Chama a funcao a cada 5 segundos (tempo em us)
  timerAlarm(timer1, 10000000, true, 0);  // Chama a funcao a cada 10 segundos (tempo em us)
  /* Parametros:
     1 - Timer a ser afetado
     2 - Ajusta o alarme para chamar a funcao a cada valor especificado
     3 - Repete o alarme
     4 - Sem limites de valor para a contagem
  */

  /* Versoes de placa ate 2.0.17 
     timer0 = timerBegin(0, 80, true);   
     Inicialização do timer. Parametros :
     0 - seleção do timer a ser usado, de 0 a 3.
     80 - prescaler. O clock principal do ESP32 é 80MHz. Dividimos por 80 para ter 1us por tick.
     true - true para contador progressivo, false para regressivo
     timerAttachInterrupt(timer0, &rotTimer0, true);
     timerAttachInterrupt(timer1, &rotTimer1, true);
     Conecta à interrupção do timer:
     - timer é a instância do hw_timer
     - endereço da função a ser chamada pelo timer
     - edge=true gera uma interrupção
     timerAlarmWrite(timer0, 5000000, true);   // 5 segundos
     timerAlarmWrite(timer1, 10000000, true);  // 10 segundos
     - o timer instanciado no inicio
     - o valor em us
     - auto-reload. true para repetir o alarme de interrupcao
     //Ativa o alarme
     timerAlarmEnable(timer0);
     timerAlarmEnable(timer1);
*/
}

// Interrupcao botao verde
void IRAM_ATTR isrGreen() {  // Guarda interrupcao na RAM e nao na memoria flash (mais lenta)
  greenTime = millis();
  if (greenTime - greenLast > 300) {  // Compara o intervalo entre apertos dos botoes
    green.press = true;               // para evitar bouncing
    greenLast = greenTime;
  }
}

// Interrupcao do botao branco
void IRAM_ATTR isrWhite() {
  whiteTime = millis();
  if (whiteTime - whiteLast > 300) {
    white.press = true;
    whiteLast = whiteTime;
  }
}

// Interrupcao botao preto
void IRAM_ATTR isrBlack() {
  blackTime = millis();
  if (blackTime - blackLast > 300) {
    black.press = true;
    blackLast = blackTime;
  }
}

void setup() {
  Serial.begin(115200);  // Inicia porta serial
  // Configuracao dos botoes
  pinMode(green.GPIO, INPUT_PULLUP);
  attachInterrupt(green.GPIO, isrGreen, FALLING);  // Botao ativo na borda de descida
  pinMode(white.GPIO, INPUT_PULLUP);
  attachInterrupt(white.GPIO, isrWhite, FALLING);
  pinMode(black.GPIO, INPUT_PULLUP);
  attachInterrupt(black.GPIO, isrBlack, FALLING);
  startTimer();  // Inicia timers da ESP32
}

void loop() {
  if (intTimer0) {
    intTimer0 = false;
    Serial.println("Interrupcao do Timer 0!");
  }
  if (intTimer1) {
    intTimer1 = false;
    Serial.println("Interrupcao do Timer 1!");
  }
  if (green.press) {
    green.press = false;
    Serial.println("Botao verde pressionado!");
  }
  if (white.press) {
    white.press = false;
    Serial.println("Botao branco pressionado!");
  }
  if (black.press) {
    black.press = false;
    Serial.println("Botao preto pressionado!");
  }
}
