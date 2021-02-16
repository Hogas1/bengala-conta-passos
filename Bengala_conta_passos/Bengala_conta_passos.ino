#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//Dados para conexão WiFi
const char* ssid = "MapleStory";
const char* password = "ruinamaple";

//Dados para conexão ao broker MQTT
#define mqtt_server "mqtt.tago.io"
#define mqtt_porta "1883"
#define mqtt_usuario "Token"
#define mqtt_senha "0b34603e-9ce3-4e4a-8c79-d02d08c3d02a"
#define mqtt_topico_pub "bengala"

#define MQTT_KEEPALIVE 65365 //Tempo em que a conexão será mantida em segundos
#define MQTT_SOCKET_TIMEOUT 65365
#define pinoSensor A0 //PINO DIGITAL UTILIZADO PELO SENSOR
#define led D4 //Pino do led embutido na placa

WiFiClient espClient;
PubSubClient client(espClient);


//Define variaveis para contagem de passos
int contador=0;
const char* num_passos=0;
char* buf="";
boolean enviou=true;

//Define variaveis para determinar se o usuário está andando/parado
unsigned long tempo_ultimo_passo = 0;
boolean andar=false;

int clck=0; //Contador de repetições da função loop


void setup(){
  pinMode(pinoSensor, INPUT); //DEFINE O PINO COMO ENTRADA
  pinMode(led, OUTPUT); //Define pino como saida

  conecta_wifi(); //Conecta com o WiFi

  //Conecta ao broker MQTT
  client.setServer(mqtt_server,1883); 
  client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha);
  client.setKeepAlive(MQTT_KEEPALIVE);
  client.setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

void loop(){
 //Verifica quando um passo é dado e chama função para contagem de passos
  if((analogRead(pinoSensor))>=85) {
    cont_passos();
  }  
  parado();//Chama função que verifica se está andando/parado

  client.loop(); //Isso deve ser chamado regularmente para permitir que o cliente 
                 //processe as mensagens recebidas e mantenha sua conexão com o servidor.
}


//Conta o número de passos
void cont_passos(){ //Conta o número de passos
  digitalWrite(led,LOW); //Led ligada para indicar um passo dado

  contador++; //Acrescenta número de passos

  // Converte número de passo para char*
  sprintf(buf,"%d",contador); 

  tempo_ultimo_passo=millis();//Guarda tempo atual do passo  
  delay(500);//Descarta valores desnecessários do sensor
  digitalWrite(led,HIGH); // Desliga led para indicar que não há passo sendo dado
  enviou = false;
}

//Verifica se está andando, caso não esteja é refeita a conexão com o broker e enviado o número de passos dados
void parado(){
  //É indicado que o usuário está parado quando está a mais de 4s sem dar um passo.
  //É enviado a contagem de passos quando o usário está parado e é completa o número de repetições da função loop reservadas à verificação de passos.  
  if(((millis() - tempo_ultimo_passo)>=4000) && enviou==false) { 
    andar=false; //Usuário parado
    enviar();
    contador=0;
  }
}

//Conecta com o WiFi
void conecta_wifi() {
    // Inicia conexão com a rede WiFi
    WiFi.begin(ssid, password);

    //Aguarda conexão ser realizada corretamente
    while (WiFi.status() != WL_CONNECTED) {
      delay(1);
    }
}

//Envia o número de passos ao broker MQTT
void enviar(){
  //Criar mensagem no formado JSON definido no TagoIO  
  char* str1 = strdup("{\n \"variable\" : \"Passos\",\n \"value\" : ");
  char* str3 = "\n}";
  char* str2 = strdup(buf);
  strcat(str1,str2);
  strcat(str1,str3);
  num_passos = str1;

  while(enviou==false){
    //Checa conexão WiFi
    if (WiFi.status() != WL_CONNECTED) {conecta_wifi();}

    //Checa conexão com o broker
    if(!client.connected()){
      reconnect();
    }

    //Faz o publish do número de passos
    enviou = client.publish(mqtt_topico_pub,num_passos);
    delay(10);
  }
}

//Reconecta ao broker MQTT
void reconnect() {
  // Entra em um loop até que a conexão seja realizada
  while (!client.connected()) {
    if (client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha)) {
    } else {
      delay(1);
    }
  }
}
