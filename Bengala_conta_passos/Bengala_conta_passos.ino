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

#define MQTT_KEEPALIVE 600 //Tempo em que a conexão será mantida em segundos
#define MQTT_SOCKET_TIMEOUT 65365 // Tempo de conexão em um socket
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
  client.connect(mqtt_usuario,mqtt_usuario,mqtt_senha,"Entrada",0,0,"0");
  client.setKeepAlive(MQTT_KEEPALIVE);
  client.setSocketTimeout(MQTT_SOCKET_TIMEOUT);
}

void loop(){
  //Checa se esta conectado ao broker e se há dado novo para ser enviado
  if(!client.connected() && enviou==false){reconnect();} 
  
  //Checa conexão WiFi
  if(WiFi.status() != WL_CONNECTED) {conecta_wifi();}

  
 //Verifica quando um passo é dado e chama função para contagem de passos
  if((analogRead(pinoSensor))>=85) {
    cont_passos();
  }    
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


//Conecta com o WiFi
void conecta_wifi() {
    // Inicia conexão com a rede WiFi
    WiFi.begin(ssid, password);

    //Aguarda conexão ser realizada corretamente
    while (WiFi.status() != WL_CONNECTED) {
      delay(1);
    }
}

//Prepara o payload para enviar o número de passos ao broker MQTT
void enviar(){
  //Criar mensagem no formado JSON definido no TagoIO  
  char* str1 = strdup("{\n \"variable\" : \"Passos\",\n \"value\" : ");
  char* str3 = "\n}";
  char* str2 = strdup(buf);
  strcat(str1,str2);
  strcat(str1,str3);
  num_passos = str1;
}

//Reconecta ao broker MQTT e enviar número de passos utilizando a will message
void reconnect() {
  // Entra em um loop até que a conexão seja realizada
  enviar();
  while (!client.connected()) {
    if (client.connect("NodeMCU Bengala",mqtt_usuario,mqtt_senha,mqtt_topico_pub,2,1,num_passos)) {
    } else {
      delay(1);
    }
  }
  contador=0;
  enviou=true;
}
