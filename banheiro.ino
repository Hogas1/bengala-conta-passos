#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ultrasonic.h> // Include Ultrasonic library


//Variaveis para conexão WiFi
const char* ssid = "brisa-1767331";
const char* password = "1wypufoa";

//Variaveis para conexão à internet e ao broker mqtt
#define mqtt_server "mqtt.tago.io"
#define mqtt_porta "1883"
#define mqtt_usuario "Token"
#define mqtt_senha "a0c88d5c-52e5-498f-a08b-b0854ab458a0"

#define MQTT_KEEPALIVE 65365 //Tempo em que a conexão será mantida em segundos
#define MQTT_SOCKET_TIMEOUT 65365


WiFiClient espClient;
PubSubClient client(espClient);

//Varial que recebe leitura dos sensores
unsigned long millisEntrou = 0;
unsigned long millisSaiu = 0;
unsigned long millisAlerta=0;
unsigned long millisCalibrar=0;

int tempo_banheiro =0;
char* char_tempo_banheiro="";

int tempo_banheiro_alerta =0;
char* char_tempo_banheiro_alerta="";

int cont_banheiro=0;
char* char_cont_banheiro="";

const char* payload_tempo=0;
const char* payload_cont=0;

char* char_status_banheiro="";
const char* payload_status_banheiro=0;

char* char_alerta_banheiro="";
const char* payload_alerta_banheiro=0;

boolean varalerta = false;
boolean varfora_do_banheiro=true;

int distancia_default=0;

//Declarando os pinos utilizados
Ultrasonic ultrasonic(D0, D1); // D0 = Trigger / D1 = Echo


void setup() {

  //Inicia monitor serial
  Serial.begin(115200);
  millisCalibrar=millis();
  while(millis()-millisCalibrar<5000){
    distancia_default=ultrasonic.distanceRead();
    delay(100);
  }
  Serial.println(distancia_default);

  //Faz conexão à internet
  setup_wifi();

  //Conecta ao broker MQTT
  client.setServer(mqtt_server,1883); 
  client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha);
  client.setKeepAlive(MQTT_KEEPALIVE);
  client.setSocketTimeout(MQTT_SOCKET_TIMEOUT);

}
//loop:
void loop() {
  if((ultrasonic.distanceRead()*2)<distancia_default && varfora_do_banheiro==true){
    no_banheiro();
    delay(30000);
  }

  if((ultrasonic.distanceRead()*2)<distancia_default && varalerta==true){
    varfora_do_banheiro=true;
    varalerta=false;
    desliga_alerta();
    delay(30000);
  }

  client.loop();
}

void no_banheiro(){
  varalerta=false;
  varfora_do_banheiro=false;
  Serial.println("Entrou no banheiro");
  millisEntrou=millis();
  cont_banheiro++;

  reconnect();
  sprintf(char_status_banheiro,"{\n \"variable\" : \"status_banheiro\",\n \"value\" : \"Sim\"\n}");
  payload_status_banheiro = char_status_banheiro;
  client.publish("banheiro",payload_status_banheiro);
  delay(300);

  while(varfora_do_banheiro==false && varalerta==false){
    
    if((ultrasonic.distanceRead()*2)<distancia_default){
      varfora_do_banheiro=true;
    }
    Serial.println("Teste0");
    if((millis()-millisEntrou)>=1800000){
      millisAlerta=millis();
      tempo_banheiro_alerta=millisAlerta-millisEntrou;
      alerta();
      break;
    }
    delay(1);
        Serial.println("Teste1");
  }
  Serial.println("Saiu do alerta");
  
  if(varfora_do_banheiro==true && varalerta==false){
    fora_banheiro();
    delay(30000);
  }
   
}


void fora_banheiro(){
  Serial.println("Saiu do banheiro");

  reconnect();
  sprintf(char_status_banheiro,"{\n \"variable\" : \"status_banheiro\",\n \"value\" :  \"Não\"\n}");
  payload_status_banheiro = char_status_banheiro;
  client.publish("banheiro",char_status_banheiro);
  delay(300);
  
  millisSaiu=millis();
  tempo_banheiro=millisSaiu-millisEntrou;
  tempo_banheiro=tempo_banheiro/60000;
  
  sprintf(char_tempo_banheiro,"{\n \"variable\" : \"tempo_banheiro\",\n \"value\" :  %d\n}",tempo_banheiro);
  payload_tempo = char_tempo_banheiro;
  Serial.println("Chegou aqui 3");
  client.publish("banheiro",payload_tempo);
  delay(300);
  
  sprintf(char_cont_banheiro,"{\n \"variable\" : \"cont_banheiro\",\n \"value\" :  %d\n}",cont_banheiro);
  payload_cont = char_cont_banheiro;
  client.publish("banheiro",payload_cont);
  delay(300);
  cont_banheiro=0;
  
  varfora_do_banheiro=true;
}

void alerta(){
  Serial.println("Alerta");
  reconnect();
  sprintf(char_alerta_banheiro,"{\n \"variable\" : \"status_alarme_banheiro\",\n \"value\" : 1\n}");
  payload_alerta_banheiro = char_alerta_banheiro;
  client.publish("banheiro",char_alerta_banheiro);

  delay(300);
 
  varalerta = true;
}

void desliga_alerta(){
  client.disconnect();
  Serial.println("Alerta desligado");
  reconnect();

  sprintf(char_status_banheiro,"{\n \"variable\" : \"status_banheiro\",\n \"value\" :  \"Não\"\n}");
  payload_status_banheiro = char_status_banheiro;
  client.publish("banheiro",char_status_banheiro);
  delay(300);
  
  sprintf(char_alerta_banheiro,"{\n \"variable\" : \"status_alarme_banheiro\",\n \"value\" : 0\n}");
  payload_alerta_banheiro = char_alerta_banheiro;
  client.publish("banheiro",char_alerta_banheiro);
  delay(300);

  millisSaiu=millis();
  tempo_banheiro=millisSaiu-millisEntrou;
  tempo_banheiro=tempo_banheiro/60000;
  
  sprintf(char_tempo_banheiro_alerta,"{\n \"variable\" : \"tempo_banheiro\",\n \"value\" :  %d\n}",tempo_banheiro);
  payload_tempo = char_tempo_banheiro_alerta;
  client.publish("banheiro",payload_tempo);
  delay(300);
  
  
  sprintf(char_cont_banheiro,"{\n \"variable\" : \"cont_banheiro\",\n \"value\" :  %d\n}",cont_banheiro);
  payload_cont = char_cont_banheiro;
  client.publish("banheiro",payload_cont);
  delay(300);
  cont_banheiro=0;
  varalerta = false;
  
}

//Conecta com o WiFi
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Reconecta ao broker MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 5 seconds before retrying
      delay(10);
    }
  }
}
