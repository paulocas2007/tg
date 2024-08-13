/*
 * MPU6050 - Le sensor giroscopio e acelerometro e exibe dados no monitor serial
 * Autor: Paulo Cesar Menegon de Castro
 * Data de criacao: 20.02.2024
 * Data de modificacao: 03.08.2024
 * Biblioteca:  https://github.com/adafruit/Adafruit_MPU6050
 * Referencias: https://RandomNerdTutorials.com/esp32-mpu-6050-accelerometer-gyroscope-arduino/
 */

// Bibliotecas
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Objeto MPU
Adafruit_MPU6050 mpu;

// Estrutura para armazenar dados
typedef struct
{
  float gx, gy, gz;  // Eixos x, y e z do giroscopio
  float ax, ay, az;  // Eixos x, y e z do acelerometro
} MPU;
MPU mpuData;

void adjustMPU() {
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

void dataMPU() {
  sensors_event_t acel, giro, temp;  // Leituras dos dados do sensor
  mpu.getEvent(&acel, &giro, &temp);
  mpuData.ax = acel.acceleration.x;  // Valores do acelerometro
  mpuData.ay = acel.acceleration.y;
  mpuData.az = acel.acceleration.z;
  mpuData.gx = giro.gyro.x;  // Valores do giroscopio
  mpuData.gy = giro.gyro.y;
  mpuData.gz = giro.gyro.z;
}

void setup(void) {
  Serial.begin(115200);  // Inicia comunicacao serial
  adjustMPU();           // Ajuste do MPU6050
}

void loop() {
  dataMPU();
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
  Serial.println("Velocidade angular:");
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
  delay(1000);  // Leitura a cada 1s
}