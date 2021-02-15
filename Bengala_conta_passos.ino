 #include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Dados para conexão WiFi
const char* ssid = "MapleStory";
const char* password = "ruinamaple";

//Dados para conexão ao broker MQTT
#define mqtt_server "mqtt.tago.io"
#define mqtt_porta "1883"
#define mqtt_usuario "Token"
#define mqtt_senha "0b34603e-9ce3-4e4a-8c79-d02d08c3d02a"
#define mqtt_topico_sub "vibracao"

#ifndef MQTT_KEEPALIVE
#define MQTT_KEEPALIVE 99999999
#endif

 // MQTT_SOCKET_TIMEOUT: socket timeout interval in Seconds
#ifndef MQTT_SOCKET_TIMEOUT
#define MQTT_SOCKET_TIMEOUT 99999999
#endif

WiFiClient espClient;
PubSubClient client(espClient);

//Define variaveis para contagem de passos
int contador=0;
const char* cont2=0;
char* buf="";
boolean enviou=false;

//Define variaveis para determinar se o usuário está andando/parado
unsigned long millisAnterior = 0;
boolean andar=false;

int clck=0; //Contador de repetições da função loop

#define pinoSensor A0 //PINO DIGITAL UTILIZADO PELO SENSOR
#define led D4 //Pino do led embutido na placa

void setup(){
  pinMode(pinoSensor, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(led, OUTPUT); //Define pino como saida
  Serial.begin(115200); //Inicia monitor serial
  setup_wifi(); //Conecta com o WiFi

  //Conecta ao broker MQTT
  client.setServer(mqtt_server,1883); 
  client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha,mqtt_topico_sub,2,0,"0");
  client.setCallback(callback); 

}

void loop(){

  
   //Verifica quando um passo é dado e chama função para contagem de passos
   if((analogRead(pinoSensor))>=85) {
     cont_passos();
   }  
   andando();//Chama função que verifica se está andando/parado

   client.loop(); //Isso deve ser chamado regularmente para permitir que o cliente 
                 //processe as mensagens recebidas e mantenha sua conexão com o servidor.
}


//Conta o número de passos
void cont_passos(){ //Conta o número de passos
  digitalWrite(led,LOW); //Led ligada para indicar um passo dado
  
  contador++; //Acrescenta número de passos
  
  // Converte número de passo para char*
  sprintf(buf,"%d",contador); 
  
  millisAnterior=millis();//Guarda tempo atual do passo
  delay(500);//Descarta valores desnecessários do sensor
  digitalWrite(led,HIGH); // Desliga led para indicar que não há passo sendo dado
  enviou = false;
}

//Verifica se está andando, caso não esteja é refeita a conexão com o broker e enviado o número de passos dados
void andando(){ 
  //É indicado que o usuário está parado quando está a mais de 4s sem dar um passo.
  //É enviado a contagem de passos quando o usário está parado e é completa o número de repetições da função loop reservadas à verificação de passos.  
  if(((millis() - millisAnterior)>=4000) && enviou==false) { 
    Serial.println("parado");
    andar=false; //Usuário parado
    enviar();
    enviou=true;
    contador=0;
  }
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

void enviar(){

    char* str1 = strdup("{\n \"variable\" : \"Passos\",\n \"value\" : ");
    char* str3 = "\n}";
    char* str2 = strdup(buf);
    
    strcat(str1,str2);
    strcat(str1,str3);
    cont2 = str1;
    reconnect();

}

//Reconecta ao broker MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha,mqtt_topico_sub,2,0,cont2)) {
      Serial.println("connected");
    } else {
      delay(10);
    }
    
  }
}


void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = '\0';
  String mensagem = String((char*)payload);
  Serial.println(mensagem);
  //SUB
  if (mensagem == "FimdoDia"){
  }
}
