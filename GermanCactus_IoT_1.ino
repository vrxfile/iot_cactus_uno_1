#include "DHT.h"
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const String APIKEY = "97f31f8321a8df31ed5efbb4f3e22072d5732d1b5d075f5d3ee85f74115d1716";
const String DEVICE = "cactusDevice@vrxfile.vrxfile";

#define SERVER_UPDATE_TIME 60000  // Update Carriots data server every 60000 ms (1 minute)

#define TIMEOUT 1000 // 1 second timout

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Local IP if DHCP fails
IPAddress ip(192, 168, 1, 250);
IPAddress dnsServerIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

IPAddress carriots_server(82, 223, 244, 60);

EthernetClient client;

long timer_main = 0;
long timer_carriots = 0;

#define DHTPIN 3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds_sensors(&oneWire);

#define LightSensorPIN A1
#define TemperatureSensorPIN A2
#define MoistureSensorPIN A3

#define LEDPIN 13

float h1 = 0;
float t1 = 0;
float t2 = 0;
float t3 = 0;
float light1 = 0;
float moisture1 = 0;

// Main setup
void setup()
{
  // Serial port
  Serial.begin(19200);
  Serial.println("/* Carriots data client by Rostislav Varzar */\n");

  // Init analog PINS
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);

  // Ethernet shield
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip, dnsServerIP, gateway, subnet);
  }
  Serial.print("LocalIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("SubnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("GatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  Serial.println("");

  // DHT11
  dht.begin();

  // DS18B20
  ds_sensors.begin();
}

// Main loop
void loop()
{
  // Main timeout
  if (millis() > timer_main + SERVER_UPDATE_TIME)
  {
    // Read data from sensors
    // DHT11
    h1 = dht.readHumidity();
    t1 = dht.readTemperature();
    if (isnan(h1) || isnan(t1)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    // Analog sensors
    float sens1 = analogRead(LightSensorPIN);
    float sens2 = analogRead(TemperatureSensorPIN);
    float sens3 = analogRead(MoistureSensorPIN);
    t2 = sens2 / 1023.00 * 5 * 100.00 * 1.2;
    light1 = sens1 / 1023.00 * 100.00;
    moisture1 = sens3 / 1023.00 * 100.00;
    // DS18B20
    ds_sensors.requestTemperatures();
    t3 = ds_sensors.getTempCByIndex(0);
    // Print data from sensors
    printAllSenors();
    // Send data to servser
    sendCarriotsStream();
    // Reset timeout timer
    timer_main = millis();
  }
}

// Send IoT packet
void sendCarriotsStream()
{

  if (client.connect(carriots_server, 80))
  {
    if (client.connected())
    {
      Serial.println("Sending data to Carriots server...\n");

      // Calculating packet size
      int thisLength = 0;
      String json_data = "";
      json_data = String("{\"protocol\":\"v2\",\"device\":\""); thisLength = thisLength + json_data.length();
      json_data = String(DEVICE); thisLength = thisLength + json_data.length();
      json_data = String("\",\"at\":\"now\",\"data\":{"); thisLength = thisLength + json_data.length();
      json_data = String("\"temperature1\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(t1, 2) + "\","); thisLength = thisLength + json_data.length();
      json_data = String("\"temperature2\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(t2, 2) + "\","); thisLength = thisLength + json_data.length();
      json_data = String("\"temperature3\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(t3, 2) + "\","); thisLength = thisLength + json_data.length();
      json_data = String("\"humidity1\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(h1, 2) + "\","); thisLength = thisLength + json_data.length();
      json_data = String("\"light1\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(light1, 2) + "\","); thisLength = thisLength + json_data.length();
      json_data = String("\"moisture1\":"); thisLength = thisLength + json_data.length();
      json_data = String("\"" + String(moisture1, 2) + "\"}}"); thisLength = thisLength + json_data.length();

      Serial.println("Size of data: " + String(thisLength));
      Serial.println();

      client.println("POST /streams HTTP/1.1");
      client.println("Host: api.carriots.com");
      client.println("Accept: application/json");
      client.println("User-Agent: Arduino-Carriots");
      client.println("Content-Type: application/json");
      client.print("carriots.apikey: ");
      client.println(APIKEY);
      client.print("Content-Length: ");
      //thisLength = json_data.length();
      client.println(thisLength);
      client.println("Connection: close");
      client.println();
      //client.println(json_data);

      client.print("{\"protocol\":\"v2\",\"device\":\"");
      client.print(DEVICE);
      client.print("\",\"at\":\"now\",\"data\":{");
      client.print("\"temperature1\":");
      client.print("\"" + String(t1, 2) + "\",");
      client.print("\"temperature2\":");
      client.print("\"" + String(t2, 2) + "\",");
      client.print("\"temperature3\":");
      client.print("\"" + String(t3, 2) + "\",");
      client.print("\"humidity1\":");
      client.print("\"" + String(h1, 2) + "\",");
      client.print("\"light1\":");
      client.print("\"" + String(light1, 2) + "\",");
      client.print("\"moisture1\":");
      client.println("\"" + String(moisture1, 2) + "\"}}");

      delay(1000);

      timer_carriots = millis();
      while ((client.available() == 0) && (millis() < timer_carriots + TIMEOUT));

      while (client.available() > 0)
      {
        char inData = client.read();
        Serial.print(inData);
      }
      Serial.println("\n");

      client.stop();
    }
  }
}

// Print sensors data to terminal
void printAllSenors()
{
  Serial.print("Temperature1: ");
  Serial.print(t1);
  Serial.println(" *C");
  Serial.print("Temperature2: ");
  Serial.print(t2);
  Serial.println(" *C");
  Serial.print("Temperature3: ");
  Serial.print(t3);
  Serial.println(" *C");
  Serial.print("Humidity1: ");
  Serial.print(h1);
  Serial.println(" %");
  Serial.print("Light detection : ");
  Serial.print(light1);
  Serial.println(" %");
  Serial.print("Moisture detection : ");
  Serial.print(moisture1);
  Serial.println(" %");
  Serial.println("");
}

