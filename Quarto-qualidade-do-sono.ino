#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>


//Variaveis para conexão à internet e ao broker mqtt
const char* ssid = "brisa-1767331";
const char* password = "1wypufoa";

//Dados para conexão ao broker MQTT
#define mqtt_server "mqtt.tago.io"
#define mqtt_porta "1883"
#define mqtt_usuario "Token"
#define mqtt_senha "b9653d7c-7387-4a00-8e8a-bfbea463768d"

#define MQTT_KEEPALIVE 65365 //Tempo em que a conexão será mantida em segundos
#define MQTT_SOCKET_TIMEOUT 65365

WiFiClient espClient;
PubSubClient client(espClient);

//Varial que recebe leitura dos sensores
int sons_por_noite=0;
int temp_atual;
int umid_atual;
int mov_por_noite=0;
boolean status_deitado = false;


//Variaveis utilizadas para envio ao broker
char* char_tempo_deitado="";
char* char_sons_por_noite="";
char* char_temp_atual="";
char* char_umid_atual="";
char* char_mov_por_noite="";
const char* payload_tempo_deitado=0;
const char* payload_som=0;
const char* payload_temp=0;
const char* payload_umid=0;
const char* payload_mov=0;

//Contador de número de execuções
int clk=0;

float tempo_deitado = 0;

boolean enviou=true;


//Portas utlizadas
#define som D4
#define deitado A0
#define movimento D0
#define dhtpin D3

#define DHTTYPE DHT11
DHT dht(dhtpin, DHTTYPE);


void setup() {
  //Define portas de entrada
  pinMode(som, INPUT);
  pinMode(deitado, INPUT);
  pinMode(movimento,INPUT);
  pinMode(dhtpin,INPUT);
  dht.begin();
  //pinMode(dormir, INPUT);

  //Inicia monitor serial
  Serial.begin(115200);

  //Faz conexão à internet
  setup_wifi();

  //Faz conexão ao broker MQTT
  client.setServer(mqtt_server,1883);
  client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha);
  client.setKeepAlive(MQTT_KEEPALIVE);
  client.setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

void loop() {
  //Reinicia a contagem de execuções
  clk=0;

  if(analogRead(deitado)>=100 && status_deitado==false){
    status_deitado=true;
    reconnect();
    envia_status_deitado();
  }

  //Realiza leitura no sensores
  if(status_deitado==true){
    tempo_deitado=millis();
    enviou=false;
    reconnect();
    envia_status_deitado();
    delay(10);
    sensores();
  }
  delay(10);
  
  if(enviou==false){
    reconnect();
    envia_status_deitado();
    envia();
    enviou=true;
  }

}

void sensores(){
  while(clk<=50000){
    //Leitura do sensor de som
    if((digitalRead(som))==1){
      sons_por_noite++;
      Serial.println("Som detectado");
      delay(50);
    }
    //Leitura do sensor de movimento
    if((digitalRead(movimento))==1){
      delay(500);
      mov_por_noite++;
      Serial.println("Movimento detectado");
    }
    //Acresenta contador de execução
    clk++;
    delay(1);
  }

  //Leitura do sensor de temperatura e umidade
  tempo_deitado=(millis()-tempo_deitado)/60000;
  temp_atual=dht.readTemperature();
  umid_atual= dht.readHumidity();
  Serial.println("Sensores-termino");
}

void envia_status_deitado(){
    client.publish("Quarto","{\n \"variable\" : \"status_deitado\",\n \"value\" :  Deitado na cama\n}");
    delay(300);
}

void envia(){
  sprintf(char_tempo_deitado,"{\n \"variable\" : \"tempo_deitado\",\n \"value\" :  %f\n}",tempo_deitado);
  payload_tempo_deitado = char_tempo_deitado;
  client.publish("Quarto",payload_tempo_deitado);
  delay(300);
  
  sprintf(char_sons_por_noite,"{\n \"variable\" : \"som\",\n \"value\" :  %d\n}",sons_por_noite);
  payload_som = char_sons_por_noite;
  client.publish("Quarto",payload_som);
  delay(300);

  sprintf(char_temp_atual,"{\n \"variable\" : \"temp\",\n \"value\" :  %d\n}",temp_atual);
  payload_temp = char_temp_atual;
  client.publish("Quarto",payload_temp);
  delay(300);

  sprintf(char_umid_atual,"{\n \"variable\" : \"umid\",\n \"value\" :  %d\n}",umid_atual);
  payload_umid = char_umid_atual;
  client.publish("Quarto",payload_umid);
  delay(300);

  mov_por_noite=(mov_por_noite/6);
  sprintf(char_mov_por_noite,"{\n \"variable\" : \"mov\",\n \"value\" :  %d\n}",mov_por_noite);
  payload_mov = char_mov_por_noite; 
  client.publish("Quarto",payload_mov);
  delay(300);

  tempo_deitado=0;
  sons_por_noite=0;
  temp_atual=0;
  umid_atual=0;
  mov_por_noite=0;
}

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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha)) {
      Serial.println("connected");
      // ... and resubscribe
      //client.subscribe(mqtt_topico_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 5 seconds before retrying
      delay(10);
    }
  }
}
