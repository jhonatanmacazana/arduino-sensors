#include "DHT.h"
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

const int DHTPin = 5;     // what digital pin we're connected to

DHT dht(DHTPin, DHTTYPE);

void setup() {
   Serial.begin(9600);
   Serial.println("Ejemplo de uso del sensor DHT22");
   dht.begin();
}

void loop() {
   delay(2000);

   float h = dht.readHumidity();
   float t = dht.readTemperature();
   if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
   }
   Serial.print("Humidity: ");
   Serial.print(h);
   Serial.print(" %\t");
   Serial.print("Temperature: ");
   Serial.print(t);
   Serial.println(" *C ");
}
