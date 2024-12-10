// Configuração inicial
void setup() {
  // Inicializa a comunicação serial com a velocidade de 115200 bps
  Serial.begin(115200);
  // Gera uma semente aleatória baseada em ruído analógico
  randomSeed(analogRead(0));
}

// Loop principal
void loop() {
  // Gera dois números aleatórios entre 0 e 100
  float valorRandom1 = random(0, 101);
  float valorRandom2 = random(0, 101);

  // Envia os valores para o monitor serial
  Serial.print("Velocidade: ");
  Serial.print(valorRandom1);
  Serial.print(" Km/H, ");
  Serial.print("Distancia: ");
  Serial.print(valorRandom2);
  Serial.println(" Cm");

  // Aguarda 1 segundo antes de gerar outros valores
  delay(1000);
}
