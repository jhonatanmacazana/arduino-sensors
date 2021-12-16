#include <Arduino.h>

const int sensorPin                   = A0;
const int ledPin                      = 13;
const int tiempoMuestraSegundos       = 2;
const int tiempoVisualizacionSegundos = 10;

int valorSensor = 0;

volatile int timerSegundosTranscurridosMuestra       = 0;
volatile int timerSegundosTranscurridosVisualizacion = 0;
boolean flagMuestra                                  = false;
boolean flagVisualizacion                            = false;
int datos[100]  = {0};  // Array para almacenar la hora de los pulsos
int indiceDatos = 0;    // Indice del array

void setup() {
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);

  setupDisplay();
  setupTimer();
}

void loop() {
  if (flagMuestra) {
    valorSensor        = analogRead(sensorPin);
    datos[indiceDatos] = valorSensor;
    indiceDatos += 1;
    flagMuestra = 0;
    Serial.println("muestra");
    Serial.println(valorSensor);
  }

  if (flagVisualizacion) {
    Serial.println("vis");
    int acumulado = 0;
    for (byte i = 0; i < indiceDatos; i += 1) {
      acumulado += datos[i];
    }
    Serial.print("Suma acumulada: ");
    Serial.println(acumulado);
    acumulado = acumulado / indiceDatos;

    acumulado = acumulado / 128;
    Serial.print("Parámetro escalado: ");
    Serial.println(acumulado);
    muestraDisplay(acumulado);

    indiceDatos       = 0;
    flagVisualizacion = 0;
  }

  blinkLed(200);
}

ISR(TIMER1_COMPA_vect) {  // Timer en cada segundo
  if (timerSegundosTranscurridosMuestra == tiempoMuestraSegundos) {
    flagMuestra                       = true;
    timerSegundosTranscurridosMuestra = 0;
  } else {
    timerSegundosTranscurridosMuestra += 1;
  }
  if (timerSegundosTranscurridosVisualizacion == tiempoVisualizacionSegundos) {
    flagVisualizacion                       = true;
    timerSegundosTranscurridosVisualizacion = 0;
  } else {
    timerSegundosTranscurridosVisualizacion += 1;
  }
}

const int displayA = 6;
const int displayB = 7;
const int displayC = 8;
const int displayD = 9;
const int displayE = 10;
const int displayF = 11;
const int displayG = 12;

const int displayLUT[10][7] = {
    {0, 0, 0, 0, 0, 0, 1},  // 0
    {1, 0, 0, 1, 1, 1, 1},  // 1
    {0, 0, 1, 0, 0, 1, 0},  // 2
    {0, 0, 0, 0, 1, 1, 0},  // 3
    {1, 0, 0, 1, 1, 0, 0},  // 4
    {0, 1, 0, 0, 1, 0, 0},  // 5
    {0, 1, 0, 0, 0, 0, 0},  // 6
    {0, 0, 0, 1, 1, 1, 1},  // 7
    {0, 0, 0, 0, 0, 0, 0},  // 8
    {0, 0, 0, 1, 1, 0, 0}   // 9
};

void setupDisplay() {
  for (int i = 0; i < 7; i++) {
    pinMode(displayA + i, OUTPUT);
  }
}

void setupTimer() {
  //   https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
  noInterrupts();  // disable all interrupts

  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A  = 15625;                       // compare match register 16MHz/1024/1Hz
  TCCR1B |= (1 << WGM12);               // modo CTC
  TCCR1B |= (1 << CS12) | (1 << CS10);  // preescalador = 1024
  TIMSK1 |= (1 << OCIE1A);              // enable timer compare interrupt

  interrupts();  // enable all interrupts
}

void muestraDisplay(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(displayA + i, displayLUT[num][i]);
  }
}

void blinkLed(long int espera) {
  digitalWrite(ledPin, HIGH);
  delay(espera);
  digitalWrite(ledPin, LOW);
  delay(espera);
}
