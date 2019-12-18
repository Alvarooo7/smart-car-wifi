#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> 
#include <Hash.h>
#include <FS.h>
#include <FirebaseArduino.h>
//DHT 
#include <DHT.h>
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
 
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);
const char* ssid = "XXXXXX";
const char* password = "XXXXXX";

#define TRIGGER 16  //D0
#define ECHO    5   //D1

long duracion=0;
int16_t distancia= 0; 
int16_t ultimoValor = 0; 
uint8_t contador = 0;  

float temperatura = 0;
float ultimaTemperatura = 0;

String msj="";
/*Movimiento */
int velocidadMovimiento =0;

/* Validad que solo un usuario controle el carro */
int estadoControl=0;
// Valida envio
short estadoChoque=0;

/* SECCIÓN MOTOR*/
//Motor Derecha
int OUTPUT4 = 14;
int OUTPUT3 = 12;
//MOTOR IZQUIERDA
int OUTPUT2 = 4;
int OUTPUT1 = 0;

WebSocketsServer webSocketMotor = WebSocketsServer(82);

WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

//Firebase
#define FIREBASE_HOST "XXXXXX"
#define FIREBASE_AUTH "XXXXXX"


//Client Envio Alerta
//WiFiClient client;

void setup(void){
  delay(1000);  
  Serial.begin(115200);

  //PINES MOTOR IZQ
  pinMode (OUTPUT1, OUTPUT);
  pinMode (OUTPUT2, OUTPUT);
 //PINES MOTOR DER
  pinMode (OUTPUT4, OUTPUT);
  pinMode (OUTPUT3, OUTPUT);
  /* Inicializando DHT */
  dht.begin();
  /* Inicializando Pines Ultra */
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);

  /* Inicializando Server */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress myIP = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(myIP);
  
  SPIFFS.begin();
  webSocketMotor.begin();
  webSocketMotor.onEvent(webSocketEventMotor); 
  
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  server.onNotFound([](){
    if(!handleFileRead(server.uri()))
      server.send(404, "text/plain", "Archivo no encontrado");
  });
  
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop(void) {
  
  webSocketMotor.loop();
  webSocket.loop();
  server.handleClient();
  contador++;
      if (contador == 255) {
        contador = 0; 
        /* ULTASONICO */
        digitalWrite(TRIGGER, LOW);  
        delayMicroseconds(2);     
        digitalWrite(TRIGGER, HIGH);
        delayMicroseconds(10);    
        digitalWrite(TRIGGER, LOW);
        duracion = pulseIn(ECHO, HIGH);
        distancia = (duracion/2) / 29.15;    
        if (distancia < 0) distancia = 0;
          
       //DHT 
       //Leemos la temperatura en grados centígrados (por defecto)
       temperatura = dht.readTemperature();
       if ( isnan(temperatura)) temperatura=0;

          
        //Mandando datos recolectados
       if (distancia != ultimoValor || temperatura != ultimaTemperatura) {
           msj = String(distancia)+"/"+String(temperatura)+"/"+String(velocidadMovimiento)+"/"+String(estadoControl);
           webSocket.broadcastTXT(msj);  
        }


        if(estadoControl==2){pilotoAutomatico();}
        else{ 
          if((distancia<15 && velocidadMovimiento==17) || temperatura>30) {
            parar();
            if(distancia<=6 && estadoChoque==0 )  almacenarDatosChoque();
            else if ((distancia>6 && estadoChoque==1) ) estadoChoque=0; 
            }
          }
        
        ultimoValor = distancia;
        ultimaTemperatura = temperatura; 
          
      }

}


void almacenarDatosChoque(){

   String usuario = Firebase.getString("task/1/name");
   Firebase.pushString("log/crack", "Accidente con "+usuario);
  // handle error
  if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());  
      return;
  }
  estadoChoque=1;
  }

void pilotoAutomatico (){

    if(temperatura>30){parar();}
    else if(distancia<15  && temperatura<31){
      parar();
      delay(200);
      atras();
      delay(2000);
      izquierda();
      delay(3000);
    }
    else{
    adelante();  
    }
  }


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch(type) {
    //En caso de que un cliente se desconecte del websocket
    case WStype_DISCONNECTED: {
      Serial.printf("Usuario #%u - Desconectado\n", num);
      break;
    }
    //Cuando un cliente se conecta al websocket presenta la información del cliente conectado, IP y ID
    case WStype_CONNECTED: {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("Nueva conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);
      String msj = String(ultimoValor);
      webSocket.broadcastTXT(msj);
      break;
    }
    //Caso para recibir información que se enviar vía el servidor Websocket
    case WStype_TEXT: {
      String entrada = "";
      for (int i = 0; i < lenght; i++) {
        entrada.concat((char)payload[i]);
      }
     estadoControl=entrada.toInt();
     if(estadoControl==0){parar();}
      break;
    }    
  }
}


//Funcion predefinida de un WebSocket
void webSocketEventMotor(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch(type) {
    //En caso de que un cliente se desconecte del websocket
    case WStype_DISCONNECTED: {
      /*Serial.printf("Usuario2 #%u - Desconectado\n", num);*/
      estadoControl=0;
      break;
    }
    //Cuando un cliente se conecta al websocket presenta la información del cliente conectado, IP y ID
    case WStype_CONNECTED: {
      estadoControl=1;
      /*IPAddress ip = webSocketMotor.remoteIP(num);
      Serial.printf("Nueva2 conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);*/
      break;
    }
    //Caso para recibir información que se enviar vía el servidor Websocket
    case WStype_TEXT: {
       String entrada = "";
       //Se lee la entrada de datos y se concatena en la variable String entrada
      for (int i = 0; i < lenght; i++) {
        entrada.concat((char)payload[i]);
      }
      //Se separan los datos de la posicion X y Y del JoyStick
      String data=entrada;
      //Serial.print(data);
      if(data){
        int pos = data.indexOf(':');
        long x = data.substring(0, pos).toInt();
        long y = data.substring(pos+1).toInt();
        //Imprime en Monitor Serial
       /* Serial.print("x:");
        Serial.print(x);
        Serial.print(", y:");
        Serial.println(y);*/
        //De acuerdo al valor de X y Y del JoyStick, se ejecuta la funcion para habilitar los motores
        if(((x<=50&&x>-50)&&(y<=50&&y>-50))){//PARAR
          parar();velocidadMovimiento=0;
        }else if(x>50&&(y<50||y>-50)){//DERECHA
          derecha();velocidadMovimiento=15;
        }else if(x<-50&&(y<50||y>-50)){//IZQUIERDA
          izquierda();velocidadMovimiento=15;
        }else if(y>50&&(x<50||x>-50)){// ADELANTE
          velocidadMovimiento=17;
          if(distancia>15) {adelante(); }
          else{ parar(); }
        }else if(y<-50&&(x<50||x>-50)){//ATRAS
          velocidadMovimiento=12;
          atras();
        }
      }
      break;
    }    
  }
}

//FUNCION PARA IDENTIFICAR EL TIPO DE CONTENIDO DE LOS ARCHIVOS DEL SERVIDOR WEB 
String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//FUNCION PARA CARGAR EL ARCHIVO DEL SERVIDOR WEB index.html
bool handleFileRead(String path){
  #ifdef DEBUG
    Serial.println("handleFileRead: " + path);
  #endif
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}

//FUNCIONES PARA ACCIONAR LOS MOTORES DE ACUERDO A LOS VALORES QUE SE ENVIAN POR MEDIO DEL JOYSTICK
void adelante(){
  // Serial.println("adelante");
  digitalWrite(OUTPUT1, 1);
  digitalWrite(OUTPUT2, 0);
  digitalWrite(OUTPUT3, 0);
  digitalWrite(OUTPUT4, 1);
   
 
}
void atras(){
  Serial.println("atras");
  digitalWrite(OUTPUT1, 0);
   digitalWrite(OUTPUT2, 1);
   digitalWrite(OUTPUT3, 1);
   digitalWrite(OUTPUT4, 0);
}

void derecha(){
  Serial.println("derecha");
  digitalWrite(OUTPUT1, 0);
  digitalWrite(OUTPUT2, 0);
  digitalWrite(OUTPUT3, 1);
  digitalWrite(OUTPUT4, 0);
}

void izquierda(){
  Serial.println("izquierda");
  digitalWrite(OUTPUT1, 0);
  digitalWrite(OUTPUT2, 1);
  digitalWrite(OUTPUT3, 0);
  digitalWrite(OUTPUT4, 0);
}
void parar(){
  Serial.println("parar");
  digitalWrite(OUTPUT1, 0);
  digitalWrite(OUTPUT2, 0);
  digitalWrite(OUTPUT3, 0);
  digitalWrite(OUTPUT4, 0);
}
