/* Copyright 2017, 2018 David Conran
*
* An IR LED circuit *MUST* be connected to the ESP8266 on a pin
* as specified by kIrLed below.
*
* TL;DR: The IR LED needs to be driven by a transistor for a good result.
*
* Suggested circuit:
*     https://github.com/crankyoldgit/IRremoteESP8266/wiki#ir-sending
*
* Common mistakes & tips:
*   * Don't just connect the IR LED directly to the pin, it won't
*     have enough current to drive the IR LED effectively.
*   * Make sure you have the IR LED polarity correct.
*     See: https://learn.sparkfun.com/tutorials/polarity/diode-and-led-polarity
*   * Typical digital camera/phones can be used to see if the IR LED is flashed.
*     Replace the IR LED with a normal LED if you don't have a digital camera
*     when debugging.
*   * Avoid using the following pins unless you really know what you are doing:
*     * Pin 0/D3: Can interfere with the boot/program mode & support circuits.
*     * Pin 1/TX/TXD0: Any serial transmissions from the ESP8266 will interfere.
*     * Pin 3/RX/RXD0: Any serial transmissions to the ESP8266 will interfere.
*   * ESP-01 modules are tricky. We suggest you use a module with more GPIOs
*     for your first time. e.g. ESP-12 etc.
*/
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Mitsubishi.h>

const uint16_t kIrLed = 12;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRMitsubishiAC ac(kIrLed);  // Set the GPIO used for sending messages.
IRMitsubishiAC ac2(kIrLed);  // Set the GPIO used for sending messages.

String state;


// Wifi nonsense
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Set these to your desired credentials. */
const char *ssid = "SINGTEL-C28F";  //ENTER YOUR WIFI SETTINGS
const char *password = "0015909519";

//Web/Server address to read/write from 
const char *host = "20.191.144.25";  

void printState() {
  // Display the settings.
  Serial.println("Mitsubishi A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  // Display the encoded IR sequence.
  unsigned char* ir_code = ac.getRaw();
  Serial.print("IR Code: 0x");
  for (uint8_t i = 0; i < kMitsubishiACStateLength; i++)
    Serial.printf("%02X", ir_code[i]);
  Serial.println();
}

void setup() {
  // Remote setup
  ac.begin();
  Serial.begin(115200);
  delay(200);

  // Set up what we want to send. See ir_Mitsubishi.cpp for all the options.
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  ac.on();
  ac.setFan(1);
  ac.setMode(kMitsubishiAcCool);
  ac.setTemp(26);
  ac.setVane(kMitsubishiAcVaneAuto);

  ac2.begin();
  ac2.off();
  ac2.setFan(1);
  ac2.setMode(kMitsubishiAcCool);
  ac2.setTemp(26);
  ac2.setVane(kMitsubishiAcVaneAuto);

  // Wifi setup
  delay(1000);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  
}

void loop() {
  // Sending GET request every 5 seconds
  HTTPClient http;    //Declare object of class HTTPClient
  String ADCData, station, getData, Link, payload;
  int adcvalue=analogRead(A0);  //Read Analog value of LDR
  ADCData = String(adcvalue);   //String to interger conversion
  station = "B";

  //GET Data
  Link = "http://20.191.144.25:5000/status/";
  
  http.begin(Link);     //Specify request destination
  
  int httpCode = http.GET();            //Send the request
  payload = http.getString();    //Get the response payload


  if (payload == "true\n"){
    if (state != payload){
      Serial.println("Sending IR command to A/C ...");
      ac.send();
      state = payload;
    }
  } 
  if (payload == "false\n"){
    if (state != payload){
      Serial.println("Trying to turn A/C off ...");
      ac2.send();
      state = payload;
    }
  }
  http.end();  //Close connection
  delay(5000*6);  //GET Data at every 5 seconds
}
