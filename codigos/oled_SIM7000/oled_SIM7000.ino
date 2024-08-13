/*
 * Funcao: Uso do display OLED
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Bibliotecas: https://github.com/adafruit/Adafruit_SSD1306
 *              https://github.com/adafruit/Adafruit-GFX-Library
 * Referencias: https://randomnerdtutorials.com/esp32-ssd1306-oled-display-arduino-ide/
 *              
 */

// Bibliotecas
#include <Wire.h>              // Comunicacao I2C
#include <Adafruit_GFX.h>      // Controle do display OLED
#include <Adafruit_SSD1306.h>  // Controle do display OLED

// Constantes
#define SCREEN_WIDTH 128  // Largura do display em pixels
#define SCREEN_HEIGHT 64  // Altura do display em pixels

// Display SSD1306 display conectado ao I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Endereco I2C do display = 0x3C
    Serial.println("Falha alocacao SSD1306");
    for (;;)
      ;
  }
  display.clearDisplay();       // Limpa o display
  display.setTextSize(1);       // Tamanho do caracter
  display.setTextColor(WHITE);  // Cor do caracter
  display.println("Teste do Display");
  display.println("0123456789");
  display.println("abcdefghijklmnopqrstuvwxyz");
  display.println("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
  display.println("!@#$%¨&*()_+`{^}<>:?-=´[~];/");
  display.display();
}

void loop() {
}
