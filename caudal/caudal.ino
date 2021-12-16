const int SENSOR_PIN             = 2;    // Sensor conectado en el pin 2
const float FACTOR_DE_CONVERSION = 7.5;  // para convertir de frecuencia a
                                         // caudal

volatile int NumPulsos;  // variable para la cantidad de pulsos recibidos

void IntContarPulsos() {
  NumPulsos += 1;
}

//---Función para obtener frecuencia de los pulsos--------
int ObtenerFrecuencia() {
  int frecuencia;
  NumPulsos = 0;           // Ponemos a 0 el número de pulsos
  interrupts();            // Habilitamos las interrupciones
  delay(1000);             // Muestra de 1 segundo
  noInterrupts();          // Deshabilitamos las interrupciones
  frecuencia = NumPulsos;  // Hz(pulsos por segundo)
  return frecuencia;
}

void setup() {
  // Monitor Serial para visualizar datos
  Serial.begin(9600);
  // Pin 2 en modo lectura
  pinMode(SENSOR_PIN, INPUT);
  // Interrupcion 0 (Pin2), funcion, Flanco de subida
  attachInterrupt(0, IntContarPulsos, RISING);
}

void loop() {
  // obtenemos la Frecuencia de los pulsos en Hz
  int frecuencia = ObtenerFrecuencia();
  // calculamos el caudal en L/m
  float caudal_L_m = (float)frecuencia / FACTOR_DE_CONVERSION;
  // calculamos el caudal en L/h
  float caudal_L_h = caudal_L_m * 60;

  //-----Enviamos por el puerto serie---------------
  Serial.print("FrecuenciaPulsos: ");
  Serial.print(frecuencia, 0);
  Serial.print("Hz\tCaudal: ");
  Serial.print(caudal_L_m, 3);
  Serial.print(" L/m\t");
  Serial.print(caudal_L_h, 3);
  Serial.println("L/h");
}