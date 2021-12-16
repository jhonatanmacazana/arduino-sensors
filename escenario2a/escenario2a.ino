#include <Arduino.h>
#include <SoftwareSerial.h>

#define DEBUG true

const int interruptPin  = 2;
int tiempoEnvioSegundos = 25;

volatile int timerSegundosTranscurridos = 0;
int cuenta                              = 0;
boolean flagEnvio                       = false;
long int datos[100]                     = {0};  // Array para almacenar la hora de los pulsos
int indiceDatos                         = 0;    // Indice del array

void setup() {
  Serial.begin(9600);

  setupDisplay();
  setupTimer();

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_Pulso, RISING);

  Serial.println("Programa de lectura de pulsos");
}

void loop() {
  if (flagEnvio) {
    String payload = getPayloadFromData();
    Serial.println(payload);

    indiceDatos = 0;
    flagEnvio   = false;
  }
  muestraDisplay(indiceDatos % 10);
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

ISR(TIMER1_COMPA_vect) {  // Timer en cada segundo
  if (timerSegundosTranscurridos == tiempoEnvioSegundos) {
    flagEnvio                  = true;
    timerSegundosTranscurridos = 0;
  } else {
    timerSegundosTranscurridos += 1;
  }
}

void ISR_Pulso() {
  datos[indiceDatos] = millis();
  indiceDatos += 1;
}

String getPayloadFromData() {
  String payload = "\"updates\":[";

  for (byte i = 0; i < indiceDatos; i += 1) {
    payload += "{\"delta_t\":" + String(datos[i] / 1000) + ",\"field1\":" + String(i) + "}";
    if (indiceDatos - i != 1) {  // mientras no sea el último elemento
      payload += ",";
    }
  }
  payload += "]}";

  return payload;
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

void muestraDisplay(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(displayA + i, displayLUT[num][i]);
  }
}
