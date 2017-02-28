/*
SENSOR POT GATEWAY (Arduino Yun)


*/

#include <SoftwareSerial.h>   //Software Serial Port
#include <Bridge.h>
#include <HttpClient.h>
#include <YunClient.h>
#include <YunServer.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define RxD 9
#define TxD 8

/*#define MQTT_SERVER "192.168.1.5"
#define MQTT_PORT 1883
#define MQTT_TOPIC "outTopic"*/
#define SERVER "192.168.1.4"      //my MQTT server address
#define PORT  1122                //my MQTT server port
#define TOPIC "topicDemo"         //my MQTT topic
#define MQTT_CLIENTID "sensorPOT"

StaticJsonBuffer<250> jsonBuffer;



void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
}

YunClient yun;

PubSubClient mqtt(SERVER, PORT, callback, yun);

SoftwareSerial blueToothSerial(RxD,TxD);
bool rightMsg=false;
char* jsonValue = "{ \"gatewayId\":\"11223344AABBCCDD\", \"nodeId\":\"%s\", \"sensorId\":%d, \"sensorValue\":\"%d\"}";
//valid example string: { "gatewayId":"11223344AABBCCDD", "nodeId":"1", "sensorId":"1", "sensorValue":"12"}

int sensorId;
//int nodeId=1;
int sensorValue;
char charBuf[150];
char msg[80]="";

void setup() {
  Serial.begin(9600);
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  delay(3000);
  Bridge.begin();
  setupBlueToothConnection();
  mqtt.connect(MQTT_CLIENTID);
  Serial.println("START LOOP"); 
  
}


void loop(void) {
  mqtt.loop();
  jsonBuffer = StaticJsonBuffer<250>();
  char recvChar;
  //char msg[80]="";
  memset(&msg[0], 0, sizeof(msg));
  int i=0;
  bool newMessage=false;
  //gestione dei messaggi ricevuti via bluetooth
  while(blueToothSerial.available()){
    newMessage=true;
    //check if there's any data sent from the remote bluetooth shield
      recvChar = blueToothSerial.read();
      Serial.print(recvChar);
      if(recvChar=='L'){
        //setupBlueToothConnection();
      }
      msg[i]=recvChar;
      //Serial.print(msg[i]);
      delay(10);  
    i++;
  }
  msg[i]='\0';
  if(newMessage){
    newMessage=false;
   /* Serial.println();
    Serial.print("MESSAGGIO RICEVUTO1: ");  //esempio: $HUM=22$TEMP=21
    for(int j=0;j<i;j++){
      Serial.print(msg[j]);
    }*/
    Serial.println();
    if (strstr(msg, "LINK LOSS") != NULL) {
      Serial.println("Persa connessione: resetto il modulo bluetooth");
      setupBlueToothConnection();
      
    }else{
      Serial.println("Ricevuto valore da un sensore");
      JsonObject& root = jsonBuffer.parseObject(msg);
      if (root.success()){
          const char* nodeId = root["nodeId"];
          int sensorId = root["sensorId"];
          int sensorValue=root["sensorValue"];
          sendDataToServer(jsonValue, nodeId, sensorId, sensorValue);
      }else{
        Serial.println("Errore nel parsing");
        delay(5000);
      }
      
    }
  }
  
}
  

void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=SensorPot\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}


//invio dei dati a SEP
void sendDataToServer(char* jsonValue, const char* nodeId, int sensorId, int sensorValue) {
  
  //memset(&charBuf[0], 0, sizeof(charBuf));
  Serial.print("Sensor Value teorico: ");
  Serial.println(sensorValue);
  sprintf(charBuf, jsonValue,nodeId,sensorId, sensorValue);
  mqtt.publish(TOPIC, charBuf);
  Serial.print("Valore inviato al server:");
  Serial.println(charBuf);
  Serial.println();
}


