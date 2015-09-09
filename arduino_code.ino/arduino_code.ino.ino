#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <EEPROM.h>

SoftwareSerial debugPort(2, 3); // RX, TX
ESP esp(&Serial, &debugPort, 4);

MQTT mqtt(&esp);
boolean wifiConnected = false;

char * ssid = "Jamwal";
char * passkey = "jamwal1953";
char * deviceName = "Neville Longbottom";
char * mqttHost = "192.168.1.9";
int mqttPort = 1883;
char * mqttUser = "";
char * mqttPass = "";

const char * deviceId     = "sdFGHDjv7w6fd/sF=";
const char * dataTopic    = "/device/sdFGHDjv7w6fd/sF=/data";
const char * commandTopic = "/device/sdFGHDjv7w6fd/sF=/cmd";
const char * baudrate - "19200";

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

  debugPort.print("data=");
  String data = res.popString();
  debugPort.println(data);

}
void mqttPublished(void* response)
{

}

void setup() {
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
