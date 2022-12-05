/*
	@Author: Huy Hoang

	This program prints the collected data to LCD1602
	If collected data surpass the limits, alert triggers.
	There is Node-Red Dashboard for collected data,
		alerts notification and changing alert limits.
*/
#include <TimeLib.h>            // for display of time
#include <SPI.h>                // for communicating with the Ethernet Shield
#include <Ethernet.h>           
#include <PubSubClient.h>
#include <LiquidCrystal.h>      // library for the LCD
#include <dht11.h>

dht11 DHT;

#define DHT11_PIN 5
#define LM35_PIN A2
#define LDR_PIN A3
#define LED 4
//#define BUZZER 4 // not enough pin
#define BUTTON1 2
#define BUTTON2 A5
#define BUTTON3 A4
#define BUTTON4 3

// initialize the numbers of the interface pins
const int rs = A1;
const int en = A0;
const int d4 = 6;
const int d5 = 7;
const int d6 = 8;
const int d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte degreeSymbol[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
  B00000
};

// Variable for scrolling text
byte scroll = 0;

// Variable for changing mode
volatile int mode = 0;

// Variable for control the alert
volatile bool alert = 1;

// Make sure that only 1 log signal is sent for each alert
bool sig = 1;

// Alert limits
long int luxAlert = 50;
int tempAlert = 50;
int humidAlert = 90;

bool buzzer = 1;

// Variables for data
float lm35_data;
int humid;
double lux;

// Variable to hold Ethernet shield MAC address
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };

// Initialize PubSubClient function
IPAddress ip(192, 168, 1, 16);       // Address of Ethernet shield
IPAddress server(192, 168, 1, 12);   // Address of Node-red server (internal network)

// Callback function header
void callback(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
PubSubClient client(server, 1883, callback, ethClient);

// Callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // the next 3 if() statements change the limit values for alert
  if(strcmp(topic,"Alert/Lux") == 0)
  {
    // convert the payload into a long int number
    char cLuxAlert[length];
    for (int i = 0 ;i < length; i++) 
    {
      cLuxAlert[i] = (char)payload[i];
    }
    luxAlert = atol(cLuxAlert);
  }
  
  if(strcmp(topic,"Alert/Temp") == 0)
  {
    // convert the payload into a long int number
    char cTempAlert[length];
    for (int i = 0 ;i < length; i++) 
    {
      cTempAlert[i] = (char)payload[i];
    }
    tempAlert = atoi(cTempAlert);
  }
  
  if(strcmp(topic,"Alert/Humid") == 0)
  {
    // convert the payload into a long int number
    char cHumidAlert[length];
    for (int i = 0 ;i < length; i++) 
    {
      cHumidAlert[i] = (char)payload[i];
    }
    humidAlert = atoi(cHumidAlert);
  }
  
  // turn off the alert remotely
  if(strcmp(topic,"Alert/Off") == 0)
  {
      if(payload[0] == '1' && alert == 1 && (lux < luxAlert || lm35_data > tempAlert || humid > humidAlert))
    {
      alert = 0;
    }
  }
} // void callback

void setup(){
  Serial.begin(9600);   // debugging
  
  lcd.createChar(0, degreeSymbol);
  lcd.begin(16, 2);

  // Interrupt to change mode
  attachInterrupt(digitalPinToInterrupt(BUTTON1), chmod, FALLING);
  // Interrupt to turn of the alert
  attachInterrupt(digitalPinToInterrupt(BUTTON4), offAlert, FALLING);

  // for alert signals
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
//  pinMode(BUZZER, OUTPUT);
  pinMode(BUTTON2, INPUT);
  pinMode(BUTTON3, INPUT);

  Ethernet.begin(mac, ip);
  if (client.connect("arduinoClient")) {
    Serial.println("Connected");
    client.publish("Data/Status","Connected");
    client.subscribe("Alert/Lux");
    client.subscribe("Alert/Temp");
    client.subscribe("Alert/Humid");
    client.subscribe("Alert/Off");            // turning off the alert from web
  }
  else Serial.println("Error!!!");
}
  
void loop(){
  // LM35 part
  int lm35_read;
  //float lm35_data;
  lm35_read = analogRead(LM35_PIN);
  lm35_data = (float(lm35_read) / 1024 * 5000) / 10;           // temperature calculation formula
  
  // DHT11 part
  DHT.read(DHT11_PIN);
  humid = (int)DHT.humidity;
  
  // LDR part
  int ldr_read;
  ldr_read = analogRead(LDR_PIN);
  //double vol_ldr = 5*((double)(ldr_read)/1024);
  lux = 8.4852*exp(0.0072*(double)ldr_read);    // formula for lux calculation from excel graph

  if(alert == 0 && (lux >= luxAlert && lm35_data <= tempAlert && humid <= humidAlert))
  {
    alert = 1;
  }
  // DISPLAY DATA ON LCD 16*2
  if(mode == 0)
  {
    lcd.setCursor(0,0);
    lcd.print("T:");lcd.print(lm35_data,0);lcd.write(byte(0));lcd.print("C| ");
  
    lcd.print("H:");lcd.print(DHT.humidity);lcd.print("% | ");
  
    lcd.print("L:");lcd.print(lux, 0);lcd.print("   ");

    lcd.setCursor(0,1);
    lcd.print(" ");
    lcd.print(tempAlert);
    lcd.write(byte(0));
    lcd.print("C |  ");
    lcd.print(humidAlert);
    lcd.print("%  |  ");
    lcd.print(luxAlert);
    lcd.print("Lx");

    
    scroll++;
    if(3 <= scroll && scroll <= 16 && (scroll%2) == 0) lcd.scrollDisplayLeft();
    if(scroll == 23)
    {
      for(int i = 0; i < 7; i++)                  // scroll back to original position
      {
        lcd.scrollDisplayRight();
      }
      scroll = 0;
    }
    // LED alert when lux < 50
    if(alert == 1 && (lux < luxAlert || lm35_data > tempAlert || humid > humidAlert))
    {
      digitalWrite(LED, HIGH);
      client.publish("Alert/Status", "Alert ON!!!");
      if(sig == 1)
      {
        client.publish("log", '1');                 // trigger the log
        sig = 0;
      }
    }
    else 
    {
      digitalWrite(LED, LOW);
      client.publish("Alert/Status", "Normal");
      sig = 1;
    }

    // buzzer alert when humid > 90% or temp > 30°C
    /*if(lm35_data > tempAlert || DHT.humidity > humidAlert) 
    {
      buzzer = !buzzer;
      if(buzzer == 1) digitalWrite(BUZZER, HIGH);
      else if(buzzer == 0) digitalWrite(BUZZER, LOW); 
    }*/
    delay(800);
  }
  else if(mode == 1)
  {
    mode1();
  }
  else if(mode == 2)
  {
    mode2();
  }
  else if(mode == 3)
  {
    mode3();
  }
  dataToNodeRed();
  client.loop();
}
