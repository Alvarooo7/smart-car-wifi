#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <FS.h>
/**#include <FirebaseArduino.h>**/
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "XXXXXX";
const char* password = "XXXXXX";

#define TRIGGER 16  //D0
#define ECHO    5   //D1
#define LIMIT_DISTANCE 15
#define COLLISION_DISTANCE 6
#define COLLISION_OCURRED true
#define MAX_TEMPERATURE 30
#define FORWARD_SPEED 17
#define LATERAL_SPEED 15

long duration = 0;
int16_t distance = 0;
int16_t lastDistance = 0;
uint8_t counter = 0;

float temperature = 0;
float lastTemperature = 0;

int speedy = 0;

/* Estado de control del carro */
int controlState = 0;
// Valida envio
boolean hasOcurredCollision = false;

/**PINES 0 Y 4 RUEDAS left **/
/**PINES 12 Y 14 RUEDAS right **/
const int pins[] = {0, 4, 12, 14};

WebSocketsServer webSocketMotor = WebSocketsServer(82);
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266WebServer server(80);

//Firebase
#define FIREBASE_HOST "XXXXXX"
#define FIREBASE_AUTH "XXXXXX"


void setup(void) {
  delay(1000);
  Serial.begin(115200);
  motorSetup();
  temperatureSensorSetup();
  ultrasonicSensorSetup();
  internetSetup();
  serverSetUp();
  webSocketSetup();
  //Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void serverSetUp() {
  SPIFFS.begin();
  server.onNotFound([]() {
    if (!handleFileRead(server.uri()))
      server.send(404, "text/plain", "Archivo no encontrado");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void webSocketSetup() {
  webSocketMotor.begin();
  webSocketMotor.onEvent(webSocketEventMotor);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

}

void internetSetup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  IPAddress myIP = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(myIP);
}

void motorSetup() {
  for (int i = 0; i < sizeof(pins); i++)
    pinMode(pins[i], OUTPUT);
}

void ultrasonicSensorSetup() {
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT);
}

void temperatureSensorSetup() {
  dht.begin();
}

void loop(void) {
  webSocketMotor.loop();
  webSocket.loop();
  server.handleClient();
  counter++;
  if (counter == 255) {
    counter = 0;
    measureDistanceInCentimeters();
    measureTemperature();
    sendDataToClient();

    if (controlState == 2)
      goForwardAndStaySafe();
    else
      checkSafeDistanceAndTemperature();

    lastDistance = distance;
    lastTemperature = temperature;
  }

}

void checkSafeDistanceAndTemperature() {
  if ((distance < LIMIT_DISTANCE && speedy == FORWARD_SPEED) || temperature > MAX_TEMPERATURE) {
    stopper();
    checkhasOcurredCollision();
  }
}

void checkhasOcurredCollision() {
  if (distance <= COLLISION_DISTANCE && hasOcurredCollision == !COLLISION_OCURRED) almacenarDatosChoque();
  else if (distance > COLLISION_DISTANCE && hasOcurredCollision == COLLISION_OCURRED) hasOcurredCollision = !COLLISION_OCURRED;
}

void sendDataToClient() {
  //Mandando datos recolectados
  if (distance != lastDistance || temperature != lastTemperature) {
    String data = String(distance) + "/" + String(temperature) + "/" + String(speedy) + "/" + String(controlState);
    webSocket.broadcastTXT(data);
  }
}

void measureDistanceInCentimeters() {
  /* ULTASONICO */
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = duration / 58.2;
  if (distance < 0) distance = 0;
}

void measureTemperature() {
  //DHT
  //Leemos la temperature en grados centígrados (por defecto)
  temperature = dht.readTemperature();
  if (isnan(temperature)) temperature = 0;
}


void almacenarDatosChoque() {
  hasOcurredCollision = COLLISION_OCURRED;
  /*
    String usuario = Firebase.getString("task/1/name");
    Firebase.pushString("log/crack", "Accidente con "+usuario);
    // handle error
    if (Firebase.failed()) {
      Serial.print("setting /message failed:");
      Serial.println(Firebase.error());
      return;
    }
  */
}

void goForwardAndStaySafe() {
  if (temperature > MAX_TEMPERATURE)
    stopper();
  else if (distance < LIMIT_DISTANCE)
    goToLeft();
  else
    forward();
}

void goToLeft() {
  stopper();
  delay(200);
  backward();
  delay(2000);
  left();
  delay(3000);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

  switch (type) {
    //En caso de que un cliente se desconecte del websocket
    case WStype_DISCONNECTED: {
        Serial.printf("Usuario #%u - Desconectado\n", num);
        break;
      }
    //Cuando un cliente se conecta al websocket presenta la información del cliente conectado, IP y ID
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("Nueva conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);
        String value = String(lastDistance);
        webSocket.broadcastTXT(value);
        break;
      }
    //Caso para recibir información que se enviar vía el servidor Websocket
    case WStype_TEXT: {
        String entrada = "";
        for (int i = 0; i < lenght; i++)
          entrada.concat((char)payload[i]);

        controlState = entrada.toInt();
        if (controlState == 0)
          stopper();

        break;
      }
  }
}


//Funcion predefinida de un WebSocket
void webSocketEventMotor(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    //En caso de que un cliente se desconecte del websocket
    case WStype_DISCONNECTED: {
        /*Serial.printf("Usuario2 #%u - Desconectado\n", num);*/
        controlState = 0;
        break;
      }
    //Cuando un cliente se conecta al websocket presenta la información del cliente conectado, IP y ID
    case WStype_CONNECTED: {
        controlState = 1;
        /*IPAddress ip = webSocketMotor.remoteIP(num);
          Serial.printf("Nueva2 conexión: %d.%d.%d.%d Nombre: %s ID: %u\n", ip[0], ip[1], ip[2], ip[3], payload, num);*/
        break;
      }
    //Caso para recibir información que se enviar vía el servidor Websocket
    case WStype_TEXT: {
        String data = "";
        //Se lee la entrada de datos y se concatena en la variable String entrada
        for (int i = 0; i < lenght; i++) {
          data.concat((char)payload[i]);
        }
        //Se separan los datos de la posicion X y Y del JoyStick
        if (data) {
          int pos = data.indexOf(':');
          long x = data.substring(0, pos).toInt();
          long y = data.substring(pos + 1).toInt();
          executeCarMovement(x, y);
        }
        break;
      }
  }
}
//De acuerdo al valor de X y Y del JoyStick, se ejecuta la funcion para habilitar los motores
void executeCarMovement(int x, int y) {
  if (((x <= 50 && x > -50) && (y <= 50 && y > -50))) //stopper
    stopper();
  else if (x > 50 && (y < 50 || y > -50)) //right
    right();
  else if (x < -50 && (y < 50 || y > -50)) //left
    left();
  else if (y > 50 && (x < 50 || x > -50)) { // forward
    if (distance > 15)
      forward();
    else
      stopper();
  } else if (y < -50 && (x < 50 || x > -50)) //backward
    backward();
}

//FUNCION PARA IDENTIFICAR EL TIPO DE CONTENIDO DE LOS ARCHIVOS DEL SERVIDOR WEB
String getContentType(String filename) {
  if (server.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

//FUNCION PARA CARGAR EL ARCHIVO DEL SERVIDOR WEB index.html
bool handleFileRead(String path) {
#ifdef DEBUG
  Serial.println("handleFileRead: " + path);
#endif
  if (path.endsWith("/")) path += "index.html";
  if (SPIFFS.exists(path)) {
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, getContentType(path));
    file.close();
    return true;
  }
  return false;
}

/** MOVIMIENTO **/
const int forwardPin[] = {1, 0, 0, 1};
const int backwardPin[] = {0, 1, 1, 0};
const int rightPin[] = {0, 0, 1, 0};
const int leftPin[] = {0, 1, 0, 0};

//FUNCIONES PARA ACCIONAR LOS MOTORES DE ACUERDO A LOS VALORES QUE SE ENVIAN POR MEDIO DEL JOYSTICK
void forward() {
  Serial.println("forward");
  speedy = 17;
  for (int i = 0; i < sizeof(pins); i++)
    digitalWrite(pins[i], forwardPin[i]);
}

void backward() {
  Serial.println("backward");
  speedy = 12;
  for (int i = 0; i < sizeof(pins); i++)
    digitalWrite(pins[i], backwardPin[i]);
}

void right() {
  Serial.println("right");
  speedy = 15;
  for (int i = 0; i < sizeof(pins); i++)
    digitalWrite(pins[i], rightPin[i]);
}

void left() {
  Serial.println("left");
  speedy = 15;
  for (int i = 0; i < sizeof(pins); i++)
    digitalWrite(pins[i], leftPin[i]);
}
void stopper() {
  Serial.println("stopper");
  speedy = 0;
  for (int i = 0; i < sizeof(pins); i++)
    digitalWrite(pins[i], 0);
}
