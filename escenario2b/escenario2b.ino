#include <Arduino.h>
#include <SoftwareSerial.h>

#define DEBUG true

SoftwareSerial Esp(5, 4);  // RX, TX

const int interruptPin       = 2;
String WIFI_SSID             = "";
String WIFI_Password         = "";
String ThingspeakWriteAPIKey = "MHM8WTIXGSQ9J97J";
String ThingspeakChannelID   = "1585381";
int tiempoEnvioSegundos      = 25;  // 1 min

volatile int timerSegundosTranscurridos = 0;
int cuenta                              = 0;
boolean flagEnvio                       = false;
long int datos[100]                     = {0};  // Array para almacenar la hora de los pulsos
int indiceDatos                         = 0;    // Indice del array

void setup() {
  Serial.begin(9600);
  Esp.begin(115200);

  setupWifi();
  setupTimer();

  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_Pulso, RISING);

  Serial.println("Programa de lectura de pulsos y envío a Thingspeak");
}

void loop() {
  if (flagEnvio) {
    sendToThingspeak();
    indiceDatos = 0;
    flagEnvio   = false;
  }
}

String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  Esp.print(command);
  long int start = millis();
  while ((start + timeout) > millis()) {
    while (Esp.available()) {
      char c = Esp.read();
      response += c;
    }
  }
  if (debug) {
    Serial.print(response);
  }
  return response;
}

void setupWifi() {
  // Reset device
  sendData("AT+RST\r\n", 2000, DEBUG);

  // Connect to wifi network
  sendData("AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_Password + "\"\r\n", 2000, DEBUG);
  delay(3000);

  // Set device in client mode
  sendData("AT+CWMODE=1\r\n", 1500, DEBUG);
  delay(1500);

  // Check current IP
  sendData("AT+CIFSR\r\n", 1500, DEBUG);
  delay(1500);

  // Enable single connections
  sendData("AT+CIPMUX=0\r\n", 1500, DEBUG);
  delay(1500);
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
  String payload = "{\"write_api_key\":\"" + ThingspeakWriteAPIKey + "\",\"updates\":[";

  for (byte i = 0; i < indiceDatos; i += 1) {
    payload += "{\"delta_t\":" + String(datos[i] / 1000) + ",\"field1\":" + String(i) + "}";
    if (indiceDatos - i != 1) {  // mientras no sea el último elemento
      payload += ",";
    }
  }
  payload += "]}";

  return payload;
}

void sendToThingspeak() {
  // https://www.mathworks.com/help/thingspeak/bulkwritejsondata.html
  String payload = getPayloadFromData();

  String request = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n";
  sendData(request, 5000, DEBUG);

  request = "AT+CIPSEND=" + String(payload.length()) + "\r\n";
  sendData(request, 5000, DEBUG);

  request = "POST /channels/" + ThingspeakChannelID +
            "/bulk_update.json HTTP/1.1\r\nContent-Type: application/json " +
            "\r\nContent-Length: " + String(payload.length()) + "\r\n\r\n";
  /* payload; + "\r\n";*/
  sendData(request, 1000, DEBUG);

  sendData(payload + "\r\n", 3000, DEBUG);

  request = "AT+CIPCLOSE\r\n";
  sendData(request, 2000, DEBUG);
}
