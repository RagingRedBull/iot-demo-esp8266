/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Credentials.h>
// Update these with values suitable for your network.

const char *SSID = S_SSID;
const char *SSID_PASSWORD = S_SSID_PASSWORD;
const char *MQTT_URL = S_MQTT_URL;
const char *MQTT_USERNAME = S_MQTT_USERNAME;
const char *MQTT_PASSWORD = S_MQTT_PASSWORD;
const char *SUBSCRIBE_TOPIC = S_SUBSCRIBE_TOPIC;
const char *PUBLISH_TOPIC = S_PUBLISH_TOPIC;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

/*
  WiFi Related Functions
*/
void printChosenSSID()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);
}
void printWifiInfo()
{
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void printWifiConnecting()
{
while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
}
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  printChosenSSID();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, SSID_PASSWORD);
  printWifiConnecting();
  randomSeed(micros());
  printWifiInfo();
}

/*
  MQTT related functions
*/
void callback(char *topic, byte *payload, unsigned int length)
{
  char payloadString[length + 1];
  memcpy(payloadString, payload, length);
  payloadString[length] = '\0';
  Serial.println(payloadString);
  DynamicJsonDocument doc(2048);
  deserializeJson(doc, payloadString);
  bool toEnable = doc["enable"];
  Serial.println(length);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println();
  // Serial.print((char*) payload);
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    Serial.println(i);
  }
  Serial.println();
  Serial.print("To Enable: ");
  Serial.println(toEnable);
  // Switch on the LED if an 1 was received as first character
  if (toEnable)
  {
    digitalWrite(D0, HIGH); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(D0, LOW); // Turn the LED off by making the voltage HIGH
  }
}

void connect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = WiFi.macAddress();
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(PUBLISH_TOPIC, "hello world", 1);
      // ... and resubscribe
      client.subscribe(SUBSCRIBE_TOPIC);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(D0, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(MQTT_URL, 1883);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected())
  {
    connect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(PUBLISH_TOPIC, msg);
  }
}