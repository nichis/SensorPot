/*
NODO SENSORE: SENSORKEY
ARDUINO UNO

*/

#include <SoftwareSerial.h>                     // Software Serial Port
#include <dht11.h>
#include <ArduinoJson.h>

dht11 DHT;
#define DHT11_PIN 4
#define RxD 7
#define TxD 6
StaticJsonBuffer<200> jsonBuffer;    //buffer per variabile JSON contenente i valori dei sensori
JsonObject& root = jsonBuffer.createObject();

String retSymb = "+RTINQ=";                     // start symble when there's any return
String slaveName = ";SensorPot";             // caution that ';'must be included, and make sure the slave name is right.
int nameIndex = 0;
int addrIndex = 0;

String recvBuf;
String slaveAddr;

String connectCmd = "\r\n+CONN=";

SoftwareSerial blueToothSerial(RxD,TxD);

void setup()
{
    Serial.begin(9600);
    pinMode(RxD, INPUT);
    pinMode(TxD, OUTPUT);
    //delay(3000);
    setupBlueToothConnection();
    //wait 1s and flush the serial buffer
    delay(1000);
    Serial.flush();
    blueToothSerial.flush();
}

void loop()
{
    char recvChar;
    while(1)
    {
        if(blueToothSerial.available())         //check if there's any data sent from the remote bluetooth shield
        {
            recvChar = blueToothSerial.read();
            Serial.print(recvChar);
        }
        if(Serial.available())                  //check if there's any data sent from the local serial terminal, you can add the other applications here
        {
            recvChar  = Serial.read();
            blueToothSerial.print(recvChar);
        }
        
        //leggo valore DHT11
        int chk;
        chk = DHT.read(DHT11_PIN);    // READ DATA
        switch (chk){
          case DHTLIB_OK:  
                      //Serial.print("OK,\t"); 
                      break;
          case DHTLIB_ERROR_CHECKSUM: 
                      Serial.print("Checksum error,\t"); 
                      break;
          case DHTLIB_ERROR_TIMEOUT: 
                      Serial.print("Time out error,\t"); 
                      break;
          default: 
                      Serial.print("Unknown error,\t"); 
                      break;
        }
       // DISPLAT DATA
        Serial.print(DHT.humidity,1);
        Serial.print(",\t");
        Serial.println(DHT.temperature,1);
        
        //invio il valore del sensore al gateway
        root["nodeId"] = "0001";
        root["sensorId"] = "0";
        root["sensorValue"] = DHT.temperature;
        root.printTo(blueToothSerial);
        
        delay(5000);
        root["nodeId"] = "0001";
        root["sensorId"] = "1";
        root["sensorValue"] = DHT.humidity;
        root.printTo(blueToothSerial);
        //blueToothSerial.print(String(root));
        /*blueToothSerial.print("$HUM=");
        blueToothSerial.print(DHT.humidity,1);
        blueToothSerial.print("$TEMP=");
        blueToothSerial.print(DHT.temperature,1);*/
        //blueToothSerial.print("Humidity=30");
        delay(15000);
    }
}

void setupBlueToothConnection()
{
    blueToothSerial.begin(38400);                               // Set BluetoothBee BaudRate to default baud rate 38400
    blueToothSerial.print("\r\n+STWMOD=1\r\n");                 // set the bluetooth work in master mode
    blueToothSerial.print("\r\n+STNA=SensorKey\r\n");       // set the bluetooth name as "SeeedBTMaster"
    blueToothSerial.print("\r\n+STAUTO=0\r\n");                 // Auto-connection is forbidden here
    delay(2000);                                                // This delay is required.
    blueToothSerial.flush();
    blueToothSerial.print("\r\n+INQ=1\r\n");                    //make the master inquire
    Serial.println("Master is inquiring!");
    delay(2000); // This delay is required.

    //find the target slave
    char recvChar;
    while(1)
    {
        if(blueToothSerial.available())
        {
            recvChar = blueToothSerial.read();
            recvBuf += recvChar;
            nameIndex = recvBuf.indexOf(slaveName);             //get the position of slave name
            
                                                                //nameIndex -= 1;
                                                                //decrease the ';' in front of the slave name, to get the position of the end of the slave address
            if ( nameIndex != -1 )
            {
                //Serial.print(recvBuf);
                addrIndex = (recvBuf.indexOf(retSymb,(nameIndex - retSymb.length()- 18) ) + retSymb.length());//get the start position of slave address
                slaveAddr = recvBuf.substring(addrIndex, nameIndex);//get the string of slave address
                break;
            }
        }
    }
    
    //form the full connection command
    connectCmd += slaveAddr;
    connectCmd += "\r\n";
    int connectOK = 0;
    Serial.print("Connecting to slave:");
    Serial.print(slaveAddr);
    Serial.println(slaveName);
    //connecting the slave till they are connected
    do
    {
        blueToothSerial.print(connectCmd);//send connection command
        recvBuf = "";
        while(1)
        {
            if(blueToothSerial.available()){
                recvChar = blueToothSerial.read();
                recvBuf += recvChar;
                if(recvBuf.indexOf("CONNECT:OK") != -1)
                {
                    connectOK = 1;
                    Serial.println("Connected!");
                    blueToothSerial.print("Connected!");
                    break;
                }
                else if(recvBuf.indexOf("CONNECT:FAIL") != -1)
                {
                    Serial.println("Connect again!");
                    break;
                }
            }
        }
    }while(0 == connectOK);
}
