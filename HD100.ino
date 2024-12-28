#include <math.h>

// ****CONST***
#define c 3 * pow(10, 8)
#define Ft 10.525 * pow(10, 8)
#define TENSAO_PIN_HD100 21
#define ECHO_PIN 2
#define TRIG_PIN 5
#define SIGNAL_PIN 4 // Pin for FIR filter input
#define SAMPLE_RATE 1000 // Sampling rate in Hz

volatile unsigned long contadorPulsos = 0; // Contador de pulsos
unsigned long ultimoTempo = 0; // Para medir o tempo decorrido
unsigned long lastSampleTime = 0; // For consistent sampling
const unsigned long samplingInterval = 1000 / SAMPLE_RATE; // Sampling interval in ms

float frequencia = 0; // Frequência calculada
float velocidade = 0;
float duracao = 0;
float distancia = 0;
float filteredSignal = 0;

// FIR Filter parameters
#define FIR_ORDER 20
float firCoefficients[FIR_ORDER] = {-2.55609373e-03, 2.31568217e-03,-5.53435231e-18,-8.32358553e-03
,2.35696731e-02,-3.89866190e-02, 3.88473007e-02,-2.66265450e-17
,-1.18129969e-01, 6.03263612e-01, 6.03263612e-01,-1.18129969e-01
,-2.66265450e-17, 3.88473007e-02,-3.89866190e-02, 2.35696731e-02
,-8.32358553e-03,-5.53435231e-18, 2.31568217e-03,-2.55609373e-03}; // Example coefficients
float firBuffer[FIR_ORDER] = {0};

// INTERRUPT
void IRAM_ATTR contarPulso() {
  contadorPulsos++; // Incrementa o contador a cada borda de subida
}

void setup() {
  Serial.begin(115200);
  pinMode(TENSAO_PIN_HD100, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SIGNAL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100), contarPulso, RISING);
}

void loop() {
  unsigned long tempoAtual = millis(); // Obtém o tempo atual

  // Handle frequency calculation every 1 second
  if (tempoAtual - ultimoTempo >= 1000) { // 1 second
    detachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100)); // Pausa a interrupção
    frequencia = contadorPulsos;
    contadorPulsos = 0;
    ultimoTempo = tempoAtual;
    attachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100), contarPulso, RISING); // Restaura a interrupção

    // HC-SR04 distance measurement
    digitalWrite(TRIG_PIN, LOW);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(11);
    digitalWrite(TRIG_PIN, LOW);

    float duracao = pulseIn(ECHO_PIN, HIGH);

    distancia = ConvertDistance(duracao);
    velocidade = DopplerConvert(frequencia);
    // Output results
    Serial.print("Velocidade: ");
    Serial.print(velocidade);
    Serial.print(" Km/H, ");
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.print(" Cm, ");
    Serial.print("Filtered Signal: ");
    Serial.println(filteredSignal);
  }

  // Handle signal sampling and FIR filtering at consistent intervals
  if (tempoAtual - lastSampleTime >= samplingInterval) {
    lastSampleTime = tempoAtual;
    float rawSignal = analogRead(SIGNAL_PIN); // Read raw signal
    filteredSignal = applyFIRFilter(rawSignal);
  }
}

// Converte para centímetros
float ConvertDistance(float duracao) {
  return duracao / 58;
}

float DopplerConvert(float frequencia) {
  float veloc = (frequencia * c * 3600) / (Ft * 2 * 1000);
  return veloc;
}

// FIR filter implementation
float applyFIRFilter(float input) {
  // Shift the buffer
  for (int i = FIR_ORDER - 1; i > 0; i--) {
    firBuffer[i] = firBuffer[i - 1];
  }
  firBuffer[0] = input;

  // Apply FIR filter
  float output = 0;
  for (int i = 0; i < FIR_ORDER; i++) {
    output += firCoefficients[i] * firBuffer[i];
  }
  return output;
}
