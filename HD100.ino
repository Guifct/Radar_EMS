#include <math.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ****CONST***
#define ANALOG_RANGE 4096
#define TENSAO_ESP 3.3
#define c 3 * pow(10, 8)
#define Ft 10.525 * pow(10, 8)
#define TENSAO_PIN_HD100 21
#define ECHO_PIN 2
#define TRIG_PIN 5
#define PICO_PIN 18
#define SIGNAL_PIN 22 // Pin for FIR filter input
#define SAMPLE_RATE 1000 // Sampling rate in Hz


const char* ssid = "NEEC_2G";        // Substitua pelo nome da sua rede Wi-Fi
const char* password = "neecfct!";   // Substitua pela senha da sua rede Wi-Fi
const char* mqttServer = "192.168.1.64"; // Exemplo de servidor MQTT (pode ser o seu servidor Mosquitto) ipconfig no cmd
const int mqttPort = 1883;

volatile unsigned long contadorPulsos = 0; // Contador de pulsos
unsigned long ultimoTempo = 0; // Para medir o tempo decorrido
unsigned long lastSampleTime = 0; // For consistent sampling
const unsigned long samplingInterval = 1000 / SAMPLE_RATE; // Sampling interval in ms

float frequencia = 0; // Frequência calculada
float velocidade = 0;
float duracao = 0;
float distancia = 0;
float filteredSignal = 0;
float pico = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// FIR Filter parameters
#define FIR_ORDER 21
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

// Função de callback para quando uma mensagem for recebida (não utilizada neste exemplo)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em tópico: ");
  Serial.println(topic);
}

void setup() {
  Serial.begin(115200);
  pinMode(TENSAO_PIN_HD100, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SIGNAL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TENSAO_PIN_HD100), contarPulso, RISING);
  delay(1000);

  // Conectar-se à rede Wi-Fi
  Serial.println("Conectando-se ao Wi-Fi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Tentando conectar...");
  }

  Serial.println("Conectado ao Wi-Fi!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Conectar ao servidor MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Tentar conectar ao servidor MQTT
  while (!client.connected()) {
    Serial.println("Tentando conectar ao servidor MQTT...");
    if (client.connect("ESP32Client")) {  // Nome do cliente MQTT
      Serial.println("Conectado ao servidor MQTT!");
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  unsigned long tempoAtual = millis(); // Obtém o tempo atual
  client.loop();
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
    pico = ReadAnalog(PICO_PIN);
    // Output results
    Serial.print("Velocidade: ");
    Serial.print(velocidade);
    Serial.print(" Km/H, ");
    Serial.print("Distancia: ");
    Serial.print(distancia);
    Serial.print(" Cm, ");
    Serial.print("Pico: ");
    Serial.print(pico);
    Serial.println(" V");

    String payload = "radar,sampling_rate=115200,source=esp32 spl=0,Distancia=X,Velocidade=Y,Pico=Z";

    String distStr = String(distancia, 2); // 2 define o número de casas decimais
    String velStr = String(velocidade, 2);
    String picoStr = String(pico, 2);

    int xPos = payload.indexOf('X'); // Encontra a posição de 'X'
    int yPos = payload.indexOf('Y'); // Encontra a posição de 'Y'
    int zPos = payload.indexOf('Z'); // Encontra a posição de 'Y'

    if (xPos != -1) {
      payload.replace("X", distStr); // Substitui 'X' pelo valor de distância
    }

    if (yPos != -1) {
      payload.replace("Y", velStr); // Substitui 'Y' pelo valor de velocidade
    }

    if (zPos != -1) {
      payload.replace("Z", picoStr); // Substitui 'Y' pelo valor de velocidade
    }

  // Exibir o resultado no Monitor Serial
  Serial.println("String atualizada: " + payload);

    if (client.publish("ems/t1/g5", payload.c_str())) {
      Serial.println("Mensagem publicada com sucesso!");
    } else {
      Serial.println("Falha ao publicar a mensagem.");
    }
  }

  // Handle signal sampling and FIR filtering at consistent intervals
  if (tempoAtual - lastSampleTime >= samplingInterval) {
    lastSampleTime = tempoAtual;
    float rawSignal = ReadAnalog(SIGNAL_PIN); // Read raw signal
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

float ReadAnalog(int pin) {
  float valAnalog = analogRead(pin);
  float valTensao = (valAnalog / ANALOG_RANGE) * TENSAO_ESP;
  return valTensao;
}
