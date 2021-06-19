// WiFi + MQTT
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";        // SSID do wifi
const char* password = "";    // Senha do WIFI
const char* mqtt_server = ""; // Servidor do MQTT

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

// DHT 
#include <DHT.h>
#define DHTPIN 15
#define DHTTYPE DHT11

float temperatura = 0;
float umidade = 0;

DHT dht(DHTPIN, DHTTYPE);

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensagem recebida do topico: ");
  Serial.print(topic);
  Serial.print(". Mensagem: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/led") {
    Serial.print("Mudando LED para ");
    if(messageTemp == "on"){
      Serial.println("on");
      digitalWrite(LED_BUILTIN, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}


// Setup
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop até se conectar
  while (!client.connected()) {
    Serial.print("Tentando conexão com MQTT...");
    // Tenta conectar
    if (client.connect("ESP32Client")) {
       Serial.println("conectado!");
       client.subscribe("esp32/led");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      // Retenta depios de 5 segundos
      delay(5000);
    }
  }
}


void loop() {
  umidade = dht.readHumidity();
  temperatura = dht.readTemperature(false);
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    char tempString[8];
    dtostrf(temperatura, 1, 2, tempString);
    Serial.print("Temperatura: ");
    Serial.println(tempString);
    // Publica temperatura no MQTT
    client.publish("home/office/temperature", tempString);

    char humString[8];
    dtostrf(umidade, 1, 2, humString);
    Serial.print("Umidade: ");
    Serial.println(humString);
    client.publish("home/office/humidity", humString);
  }

  delay(1000);
}
