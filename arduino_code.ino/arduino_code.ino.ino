#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <EEPROM.h>

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&Serial, &debugPort, 4);

MQTT mqtt(&esp);
boolean wifiConnected = false;

char * ssid = "ITI";
char * passkey = "";
char * deviceName = "Neville Longbottom";
char * mqttHost = "192.168.1.3";
int mqttPort = 1883;
char * mqttUser = "";
char * mqttPass = "";

const char * deviceId     = "sdFGHDjv7w6fdsF";
const char * dataTopic    = "/device/sdFGHDjv7w6fdsF/data";
const char * commandTopic = "/device/sdFGHDjv7w6fdsF/cmd";
const char * baudrate = "19200";

int relayPin = 5;

struct service_data {
  unsigned char address;
  unsigned char length;
  unsigned char data;
};

struct packet {
  unsigned char version;
  unsigned char type;
  unsigned char unused[3];
  unsigned char count;
  unsigned char * raw_data;
} ;


#define RELAY(pin) pinMode(pin, OUTPUT)
#define RELAY_OFF(pin) digitalWrite(relayPin, HIGH)
#define RELAY_ON(pin) digitalWrite(relayPin, LOW)

void processServiceMessage(struct service_data * serv) {
  if(serv->data == 0) {
    debugPort.print("Turning Off");
    RELAY_OFF(relayPin);
  } else {
    debugPort.print("Turning On");
    RELAY_ON(relayPin);
  }
}

void processMessage(struct packet * pack) {
  unsigned char count = pack->count;
  unsigned char * data = pack -> raw_data;
  struct service_data *serv = (struct service_data* ) data;
  
  while(count -- > 0) {
    processServiceMessage(serv);
    data = data + 1 + serv->length;
  }
}

void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      debugPort.println("WIFI CONNECTED");
      mqtt.connect(mqttHost, mqttPort);
      wifiConnected = true;
    } else {
      wifiConnected = false;
      mqtt.disconnect();
    }
    
  }
}

void mqttConnected(void* response)
{
  debugPort.println("Connected to Mqtt");
  mqtt.subscribe(commandTopic); //or mqtt.subscribe("topic"); /*with qos = 0*/
  mqtt.publish(dataTopic, "data0");
}

void mqttDisconnected(void* response)
{

}

void mqttData(void* response)
{
  RESPONSE res(response);

  debugPort.print("Received: topic=");
  String topic = res.popString();
  debugPort.println(topic);

  String data = res.popString();
  debugPort.println(data);

  processMessage((struct packet *)data.c_str());
  
}
void mqttPublished(void* response)
{

}

void setup() {
  RELAY(relayPin);
  RELAY_OFF(relayPin);
  
  Serial.begin(19200);
  debugPort.begin(19200);
  esp.enable();
  
  delay(500);
  
  esp.reset();
  
  delay(500);
  
  debugPort.println("Device Reset. Waiting for it to get ready");
  
  while(!esp.ready());
  
  debugPort.println("ARDUINO: setup mqtt client");
  if(!mqtt.begin(deviceName, mqttUser, mqttPass, 120, 1)) {
    debugPort.println("ARDUINO: fail to setup mqtt");
    while(1);
  }


  debugPort.println("ARDUINO: setup mqtt lwt");
  mqtt.lwt("/lwt", "offline", 0, 0); //or mqtt.lwt("/lwt", "offline");

  /*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);

  /*setup wifi*/
  debugPort.println("ARDUINO: setup wifi");
  esp.wifiCb.attach(&wifiCb);

  esp.wifiConnect(ssid, passkey);

  debugPort.println("ARDUINO: system started");
}

void loop() {
  esp.process();
  if(wifiConnected) {

  }
}
