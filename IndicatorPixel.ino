/*
This sketch uses a neopixel ring as an value indicator.

It runs a tiny web server that receives commands and parameters in the url
eg. http://<ip address>/?value=200&min=100&max=300&brightness=40

The value passed in is the raw value to use
Min is the minimum to use for mapping the raw value into a mapped range of 0-100
Max is the maximum to use for mapping the raw value into a mapped range of 100

In the above example, 200 would be mapped to 50 as it sits in the middle between the min and max values

The mapped values in the 0-100 range are then mapped onto the number of leds defined

This sketch uses the FastLED v3.3.3 and the ESP8266 by ESP8266 Community board v2.6.2

Settings for Wemos D1 Mini

*/

#include <ESP8266WiFi.h>
#include <FastLED.h>

#ifndef STASSID
  #define STASSID "[Your WiFi SSID]"
  #define STAPSK  "[Your WiFi Password]"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

// How many leds are in the strip?
#define NUM_LEDS 16

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI
#define DATA_PIN 2
//#define CLOCK_PIN 13

// This is an array of leds.  One item for each led in your strip.
CRGB leds[NUM_LEDS];


void setup() {
  Serial.begin(115200);

  // Prepare LED by setting type of pixel strip, colour order and number of LEDs
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Display initialisation sequence on LEDs to show code active and LEDs working
  Initialise_Sequence();
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
/*  int val;
  if (req.indexOf(F("/gpio/0")) != -1) {
    val = 0;
  } else if (req.indexOf(F("/gpio/1")) != -1) {
    val = 1;
  } else {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }
*/

  int val;
  int mapped_val;
  int value_min;
  int value_max;
  int brightness;
  
  int urlposition;

  // Ignore the favicon GET request that comes after initial call
  if (req.indexOf(F("/favicon.ico")) != -1)
  //if (req=="GET /favicon.ico HTTP/1.1")
  {
    Serial.println("Favicon request - ignoring");
  }
  else
  {
    urlposition = req.indexOf(F("/value="));
    Serial.print("val substring before processing=");
    Serial.println(req.substring(urlposition+7,req.indexOf(F("&min="))));
    val = req.substring(urlposition+7,req.indexOf(F("&min="))).toInt();
    Serial.print("Value submitted=");
    Serial.println(val);

    urlposition = req.indexOf(F("&min="));
    Serial.print("min substring before processing=");
    Serial.println(req.substring(urlposition+5,req.indexOf(F("&max="))));
    value_min = req.substring(urlposition+5,req.indexOf(F("&max="))).toInt();
    Serial.print("Min submitted=");
    Serial.println(value_min);
    
    urlposition = req.indexOf(F("&max="));
    Serial.print("max substring before processing=");
    Serial.println(req.substring(urlposition+5,req.indexOf(F("&brightness="))));
    value_max = req.substring(urlposition+5,req.indexOf(F("&brightness="))).toInt();
    Serial.print("Max submitted=");
    Serial.println(value_max);

    urlposition = req.indexOf(F("&brightness="));
    brightness = req.substring(urlposition+12).toInt();
    Serial.print("Brightness submitted=");
    Serial.println(brightness);

    // Re-Map incoming val to percentage. remapped values can be < 0 or > 100
    mapped_val = map(val,value_min,value_max,0,100);
    
    Serial.print("Mapped value=");
    Serial.println(mapped_val);
    
    // Reset LEDs
    FastLED.clear();
    // Set LED according to the request
    SetLEDValue_reverse(mapped_val,brightness);
  }
  
  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available()) {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nValue set to "));
  client.print((val));
  client.print(F("\r\nMin used:"));
  client.print((value_min));
  client.print(F("\r\nMax used:"));
  client.print((value_max));
  client.print(F("\r\nBrightness used:"));
  client.print((brightness));
  
  client.print(F("</html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}

// ToDo: The SetLEDValue function is older than SetLEDValue_reverse and needs updating/cloning to match
void SetLEDValue(int value)
{
   // value passed in is the percent to set on the LED display. eg 50 = half the LEDs lit

   int pixelColour;
   
   for(int ledloop = 0; ledloop < NUM_LEDS-1; ledloop = ledloop + 1) {
      // Set the correct colour of this pixel accordingly
      // 0-69% = green
      // 70-89% = orange/yellow
      // 90-100% = red
      if (ledloop > map(value,0,100,0,NUM_LEDS-1))
      {
        break;
      }

      int orangepixelthreshold = map(50,0,100,0,NUM_LEDS-1);
      int redpixelthreshold = map(75,0,100,0,NUM_LEDS-1);
      Serial.print("Orange_threshold=");
      Serial.println(orangepixelthreshold);

      Serial.print("Red_threshold=");
      Serial.println(redpixelthreshold);

      if (ledloop < orangepixelthreshold)
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Green");
       
        // green
        leds[ledloop] = CHSV(96,255,30); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
      else if (ledloop >= orangepixelthreshold && ledloop < redpixelthreshold)
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Orange");
        // orange
        leds[ledloop] = CHSV(35,255,30); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
      else
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Red");
        // red
        leds[ledloop] = CHSV(0,255,30); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
            
      // Turn our current led on to white, then show the leds
      //leds[ledloop] = CRGB::White;
      //leds[ledloop] = CRGB(255,255,255);
      
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();

      // Wait a little bit
      delay(100);

      // Turn our current led back to black for the next loop around
//      leds[ledloop] = CRGB::Black;
   }
}

void SetLEDValue_reverse(int value,int brightness)
{
   // value passed in is the percent to set on the LED display. eg 50 = half the LEDs lit

   int pixelColour;
   
   for(int ledloop = 0; ledloop < NUM_LEDS; ledloop = ledloop + 1) {
      // Set the correct colour of this pixel accordingly
      // 0-50% = green
      // 51-75% = orange/yellow
      // 76-100% = red

      // If current LED > the number of LEDs we need to set then exit out
      if (ledloop > map(value,0,100,0,NUM_LEDS))
      {
        Serial.print("LEDLoop=");
        Serial.print(ledloop);
        Serial.println(" Breaking out");
        break;
      }

      // All passed in values as parameter are pre-converted to percentage 0-100, so here
      // use 0-100 as the input range for map function
      int orangepixelthreshold = map(50,0,100,0,NUM_LEDS);
      int redpixelthreshold = map(75,0,100,0,NUM_LEDS);
      Serial.print("Orange_threshold=");
      Serial.println(orangepixelthreshold);

      Serial.print("Red_threshold=");
      Serial.println(redpixelthreshold);

      if (ledloop < orangepixelthreshold)
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Green");
       
        // green
        leds[NUM_LEDS-1-ledloop] = CHSV(96,255,brightness); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
      else if (ledloop >= orangepixelthreshold && ledloop < redpixelthreshold)
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Orange");
        // orange
        leds[NUM_LEDS-1-ledloop] = CHSV(35,255,30); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
      else
      {
        Serial.print("LED[");
        Serial.print(ledloop);
        Serial.println("]=Red");
        // red
        leds[NUM_LEDS-1-ledloop] = CHSV(0,255,30); // Hue (red is 0, green is 96, blue is 160), Saturation (255 is fully saturated), Value=Brightness (10 is dim)
      }
            
      // Show the leds
      FastLED.show();

      // Wait a little bit for an animated effect as the LEDs are lit
      delay(100);

   }
}

void Initialise_Sequence()
{
  FastLED.clear();
  delay(100);
  
  for (int ledloop = 0; ledloop < NUM_LEDS ; ledloop++)
  {
    leds[NUM_LEDS-1-ledloop] = CHSV(map(ledloop,0,NUM_LEDS-1,160,0),255,128);
    FastLED.show();
    delay(50);
  }

  for (int ledloop = 0; ledloop < NUM_LEDS ; ledloop++)
  {
    leds[ledloop] = CRGB::Black;
    FastLED.show();
    delay(50);
  }
    
  delay(1000);
  FastLED.clear();
  
}
