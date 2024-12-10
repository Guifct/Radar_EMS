#include <math.h>

// ****CONST***
#define c 3*pow(10,8)
#define Ft 10,525*pow(10,8)
#define TENSAO_PIN_HD100 34
#define ECHO_PIN 35
#define TRIG_PIN 32


volatile unsigned long contadorPulsos = 0; // Contador de pulsos
unsigned long ultimoTempo = 0; // Para medir o tempo decorrido
float frequencia = 0; // Frequência calculada
float velocidade = 0;
float duracao = 0;
float distancia = 0;

// INTERRUPT
void IRAM_ATTR contarPulso() {
  contadorPulsos++; // Incrementa o contador a cada borda de subida
}

void setup() {
  Serial.begin(115200);
  pinMode(TENSAO_PIN_HD100, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100), contarPulso, RISING);
}

void loop() {
  unsigned long tempoAtual = millis(); // Obtém o tempo atual
  if (tempoAtual - ultimoTempo >= 1000) { // 1 segundo
    detachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100)); // Pausa a interrupção 
    frequencia = contadorPulsos;
    contadorPulsos = 0;
    ultimoTempo = tempoAtual;
    attachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100), contarPulso, RISING); // Restaura a interrupção

    //parte do HC-SR04
    digitalWrite(TRIG_PIN, LOW);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(11);
    digitalWrite(TRIG_PIN, LOW);

    float duracao = pulseIn(ECHO_PIN,HIGH);
    
    distancia = ConvertDistance(duracao);
    velocidade = DopplerConvert(frequencia);

    //Serial.println("HD100");
    Serial.print("Velocidade: ");
    Serial.print(velocidade);
    Serial.print(" Km/H, ");
    //Serial.print("Frequência: ");
    //Serial.print(frequencia);
    //Serial.println(" Hz");
    //Serial.println("HC-SR04");
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.println(" Cm");
    
  }
}
// converte para centimentros
float ConvertDistance(float duracao)
{
  return duracao/58;
}

float DopplerConvert(float frequencia) 
{
  float veloc = (frequencia*c*3600)/(Ft*2*1000);
  return veloc;
}
/*// **ANALOG DEF**
#define ANALOG_RANGE 4096
#define TENSAO_ESP 3.3
#define TENSAO_PIN_HD100 34



#define SEGUNDOS_ESPERA 0.5
#define NUM_AMOSTRAS 100

float ReadAnalog(int pin) {
  float valAnalog = analogRead(pin);
  float valTensao = (valAnalog / ANALOG_RANGE) * TENSAO_ESP;
  return valTensao;
}


*/