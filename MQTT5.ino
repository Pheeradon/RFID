// RFID + Node
// SDA TO D2
// RST TO D1
// MOSI TO D7
// MISO TO D6
// SCK TO D5
// 3.3V TO 3.3V
// GND TO G
// RG ------

//////////////////////////////////////////////

// LEDI2C
// VCC TO 5V
// GND TO G
// SCL TO D1
// SDA TO D2

//////////////////////////////////////////////

//include libary
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h> 
// PIN 
#define RST_PIN 5 // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN  4  // SDA-PIN for RC522 - RFID - SPI - Modul GPIO2
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

///////////////////////////////////////////////

String read_rfid; 
String mac = WiFi.macAddress();
String Jsondata="";
const char* drug;
const char* Publish = "RFID";
const char* Sublish = "drug";

//////////////////////////////////////////////

DynamicJsonBuffer jsonBuffer;
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

///////////////////////////////////////////////////////

const char* ssid = "ASD";
const char* password = "12344321";
//const char* mqtt_server = "192.168.216.128";
const char* mqtt_server = "broker.mqttdashboard.com";

//////////////////////////////////////////////////////

void dump_byte_array(byte *buffer, byte bufferSize) {
    read_rfid="";
    for (byte i = 0; i < bufferSize; i++) {
      read_rfid=read_rfid + String(buffer[i], HEX);
          }
}

//////////////////////////////////////////////////////////////

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

//////////////////////////////////////////////////////////////

void setup() {
 
 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
   SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522 
  client.setCallback(callback);

  delay(100);

}
////////////////////////////////////////////////////////////////

void loop() {
  
  if(!client.connected()){
    reconnect();
  }
  RFID();
client.loop();
}

///////////////////////////////////////////////////////////////////////

void RFID(){
   
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  
  // Select one of the cards
  
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  if(read_rfid != ""){
      Json();
      if (client.subscribe(Sublish)) {Serial.println("Subscribe Success !! "); Serial.print("JSONDATA : "); Serial.println(Jsondata);} else {Serial.println("Subscribe Fail !! ");}
  }
  }

///////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived : ");
  
  char *cstring = (char *) payload;
  cstring[length] = '\0';    // Adds a terminate terminate to end of string based on length of current payload
  Serial.println(cstring);
  Jsondata=cstring;
  dataparse();
  }
/////////////////////////////////////////////////////////////////////////
void dataparse(){
    String jsonMessage = Jsondata;
  JsonObject& root = jsonBuffer.parseObject(jsonMessage);
   if(read_rfid != ""){
      drug=root["namedrug"];
      Serial.print("DRUG NAME :");
      Serial.println(drug);
}
  }
//////////////////////////////////////////////////////////////
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.println("OK");
      //client.subscribe(Sublish);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////////////////////////////////////////////////////////////////
void Json(){
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["RFID"] = read_rfid;
  root["MAC"] = mac;
  char JSONmessageBuffer[100];
  root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
  if (client.publish(Publish,JSONmessageBuffer) == true) {
    Serial.println("Success !! ");
    } else {
      Serial.println("Fail !! ");
    }
  delay(1000);

//      if ( read_rfid != "") {
//        
////        client.subscribe(Sublish);
////        delay(1000);
//    } else {
//    Serial.println("Error sending message");
//  }  
}     
