#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Hugo's phone";        // Substitua pelo nome da sua rede Wi-Fi
const char* password = "ygoa8849";   // Substitua pela senha da sua rede Wi-Fi

// Defina o servidor MQTT (Mosquitto) e a porta
const char* mqttServer = "192.168.217.76"; // Exemplo de servidor MQTT (pode ser o seu servidor Mosquitto) ipconfig no cmd
const int mqttPort = 1883;                  // Porta padrão do Mosquitto (pode ser 1883 ou outra)

WiFiClient espClient;
PubSubClient client(espClient);

// Função de callback para quando uma mensagem for recebida (não utilizada neste exemplo)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida em tópico: ");
  Serial.println(topic);
}

void setup() {
  // Inicializa a comunicação serial
  Serial.begin(115200);
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
  // Manter a conexão com o servidor MQTT
  client.loop();

  // Enviar uma mensagem para um tópico específico
  String payload = "tomatoma";
  if (client.publish("ems/f/s", payload.c_str())) {
    Serial.println("Mensagem publicada com sucesso!");
  } else {
    Serial.println("Falha ao publicar a mensagem.");
  }

  delay(5000);  // Aguarda 5 segundos antes de publicar novamente
}

