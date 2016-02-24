#include "DHT.h"
#include "pitches.h"
#include "LedControl.h"
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

#define BUZZERPIN 5

const int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
const int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

/*
  LED Matrix
  pin A4 is connected to the DataIn
  pin A5 is connected to the CLK
  pin D7 is connected to LOAD(CS)
*/

//LedControl lc = LedControl(A4, A5, 7, 1);

/*
  // Smiles for LED matrix
  const byte PROGMEM smile_sad[8] =
  {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10011001,
  B10100101,
  B01000010,
  B00111100
  };
  const byte PROGMEM smile_neutral[8] =
  {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10000001,
  B10111101,
  B01000010,
  B00111100
  };
  const byte PROGMEM smile_happy[8] =
  {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100
  };
*/

// Main setup
void setup()
{
  // Serial port
  Serial.begin(9600);
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

  // LED matrix
  /*
    lc.shutdown(0, false);
    lc.setIntensity(0, 8);
    lc.clearDisplay(0);
  */
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
    // If moisture less than 30, play music
    if (moisture1 <= 10) {
      /*
        lc.setRow(0, 0, smile_sad[0]);
        lc.setRow(0, 1, smile_sad[1]);
        lc.setRow(0, 2, smile_sad[2]);
        lc.setRow(0, 3, smile_sad[3]);
        lc.setRow(0, 4, smile_sad[4]);
        lc.setRow(0, 5, smile_sad[5]);
        lc.setRow(0, 6, smile_sad[6]);
        lc.setRow(0, 7, smile_sad[7]);
      */
      for (int thisNote = 0; thisNote < 8; thisNote++) {
        int noteDuration = 1000 / noteDurations[thisNote];
        tone(BUZZERPIN, melody[thisNote], noteDuration);
        int pauseBetweenNotes = noteDuration * 1.30;
        delay(pauseBetweenNotes);
        noTone(BUZZERPIN);
      }
    }
    if ((moisture1 > 10) && (moisture1 <= 30)) {
      /*
        lc.setRow(0, 0, smile_neutral[0]);
        lc.setRow(0, 1, smile_neutral[1]);
        lc.setRow(0, 2, smile_neutral[2]);
        lc.setRow(0, 3, smile_neutral[3]);
        lc.setRow(0, 4, smile_neutral[4]);
        lc.setRow(0, 5, smile_neutral[5]);
        lc.setRow(0, 6, smile_neutral[6]);
        lc.setRow(0, 7, smile_neutral[7]);
      */
    }
    if (moisture1 > 30) {
      /*
        lc.setRow(0, 0, smile_happy[0]);
        lc.setRow(0, 1, smile_happy[1]);
        lc.setRow(0, 2, smile_happy[2]);
        lc.setRow(0, 3, smile_happy[3]);
        lc.setRow(0, 4, smile_happy[4]);
        lc.setRow(0, 5, smile_happy[5]);
        lc.setRow(0, 6, smile_happy[6]);
        lc.setRow(0, 7, smile_happy[7]);
      */
    }
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

