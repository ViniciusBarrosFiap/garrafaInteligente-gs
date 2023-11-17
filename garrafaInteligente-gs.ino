//Autor: Fábio Henrique Cabrini
//Rev2: 28-08-2023 Ajustes para o funcionamento no FIWARE Descomplicado
//Autor Rev2: Fábio Henrique Cabrini

#include <WiFi.h>
#include <PubSubClient.h>  // Importa a Biblioteca PubSubClient
#include <Wire.h>
//defines:
//defines de id mqtt e tópicos para publicação e subscribe denominado TEF(Telemetria e Monitoramento de Equipamentos)
#define TOPICO_SUBSCRIBE "/TEF//cmd"              //tópico MQTT de escuta
#define TOPICO_PUBLISH "/TEF/flooence/attrs"      //tópico MQTT de envio de informações para Broker
#define TOPICO_PUBLISH_2 "/TEF/flooence/attrs/d"  //tópico MQTT de envio de informações para Broker
//IMPORTANTE: recomendamos fortemente alterar os nomes
//            desses tópicos. Caso contrário, há grandes
//            chances de você controlar e monitorar o ESP32
//            de outra pessoa.
#define ID_MQTT "fiware_flooence"  //id mqtt (para identificação de sessão)
//IMPORTANTE: este deve ser único no broker (ou seja,
//            se um client MQTT tentar entrar com o mesmo
//            id de outro já conectado ao broker, o broker
//            irá fechar a conexão de um deles).
// o valor "n" precisa ser único!

#define echoPin 12  // Pino do echo
#define trigPin 13  // Pino do trig


int maxDistance = 100;
long duration, distance;
float arrayDistancia[5];  // Array para média móvel

// WIFI
const char* SSID = "ANDAR SUPERIOR";  // SSID / nome da rede WI-FI que deseja se conectar
const char* PASSWORD = "baba404040";  // Senha da rede WI-FI que deseja se conectar

// MQTT
const char* BROKER_MQTT = "46.17.108.113";  //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                     // Porta do Broker MQTT

//Variáveis e objetos globais
WiFiClient espClient;          // Cria o objeto espClient
PubSubClient MQTT(espClient);  // Instancia o Cliente MQTT passando o objeto espClient


//Prototypes
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void);

/*
    Implementações das funções
*/
void setup() {
  //inicializações:
  initSerial();
  initWiFi();
  initMQTT();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  delay(5000);
}

//Função: inicializa comunicação serial com baudrate 115200 (para fins de monitorar no terminal serial
//        o que está acontecendo.
//Parâmetros: nenhum
//Retorno: nenhum
void initSerial() {
  Serial.begin(115200);
}

//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() {
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");

  reconectWiFi();
}

//Função: inicializa parâmetros de conexão MQTT(endereço do
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);  //informa qual broker e porta deve ser conectado
  MQTT.setCallback(mqtt_callback);           //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

//Função: função de callback
//        esta função é chamada toda vez que uma informação de
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;

  //obtem a string do payload recebido
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Havera nova tentatica de conexao em 2s");
      delay(2000);
    }
  }
}

//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() {
  //se já está conectado a rede WI-FI, nada é feito.
  //Caso contrário, são efetuadas tentativas de conexão
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(SSID, PASSWORD);  // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());
}

//Função: verifica o estado das conexões WiFI e ao broker MQTT.
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void) {
  if (!MQTT.connected())
    reconnectMQTT();  //se não há conexão com o Broker, a conexão é refeita

  reconectWiFi();  //se não há conexão com o WiFI, a conexão é refeita
}


//programa principal
void loop() {
  char msgBuffer[6];



  //garante funcionamento das conexões WiFi e ao broker MQTT
  VerificaConexoesWiFIEMQTT();
  
  //Leitura da distancia com delay um pouco maior entre cada leitura
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2;

  // Média Móvel
  for (int i = 0; i < 4; i++) {
    arrayDistancia[i] = arrayDistancia[i + 1];
  }
  arrayDistancia[4] = distance;

  float media = 0;
  for (int i = 0; i < 5; i++) {
    media += arrayDistancia[i];
  }
  media /= 5;

  // Ajusta o valor de distância para 100 se for maior do que maxDistance
  if (media > 10) {
    media = 10;
  }

  // Calcula a porcentagem com base na distância medida
  int percentage = map(distance, 0, 10, 0, 10);
  dtostrf(percentage, 4, 2, msgBuffer);
  delay(1500);
  MQTT.publish(TOPICO_PUBLISH_2, msgBuffer);

  //keep-alive da comunicação com broker MQTT
  MQTT.loop();
}