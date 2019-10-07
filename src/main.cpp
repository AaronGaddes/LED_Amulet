// Import required libraries
// Make sure these have been downloaded into workspace
// in Arduino or Platform.io IDEs
// Uncomment appropriate includes depending on dev board being used

// ESP32 | ESP8266

// ############## ESP32 ##############
// #include <Arduino.h>
// #include "WiFi.h"
// #include "ESPAsyncWebServer.h"
// #include "SPIFFS.h"
// #include "FastLED.h"

// ############## ESP8266 ##############
// #include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
// #include <ESP8266WebServer.h>
#include "ESPAsyncWebServer.h"
#include <ESP8266mDNS.h>
#include <FastLED.h>
#include <FS.h>   // Include the SPIFFS library

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

// Pin attached to LED strip's data line
#define DATA_PIN        D7

// LED Strip type
#define LED_TYPE        WS2811

// LED Strip's color order
#define COLOR_ORDER     GRB

// Maximum number of LEDs.
// This is for on-the-fly calibration
// when connecting different crystals
// with different length LED strips
#define MAX_LEDS        10

// Maximum voltage and miliamp to use with the FAST_LED library
// to attempt to prevent against exces power usage
// and catastrophic failures
#define MAX_VOLTAGE     5
#define MAX_MILLI_AMPS  1000

// Create Array of LEDs
// using the Maximum number of LEDs
// expected to be used
CRGB leds[MAX_LEDS];

#define FRAMES_PER_SECOND  120

// Set the initial number of LEDs to be the maximum number
uint8_t NUM_LEDS = MAX_LEDS;

// Theses are used to control the pulse pattern
// As well as the maximum brightness of the LED strip
uint8_t MAX_BRIGHTNESS = 50;
uint8_t BRIGHTNESS = 0;
uint8_t pulseFadeAmount = 5;

// The current pattern, set by 
String currentPattern = "solid";


// Index number of which pattern is currently active
// This is used in the 'cycle' pattern so as to not override
// the current selected pattern
uint8_t gCurrentPatternNumber = 0;

// rotating "base color" used by some of the patterns
uint8_t gHue = 0;


// Network credentials
// uncomment password if needed
// and add to the 'WiFi.softAP()' function call in the setup function
const char* ssid = "ESP8266-Access-Point";
// const char* password = "123456789";

// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8);   //optional
IPAddress secondaryDNS(8, 8, 4, 4); //optional



// Set LED GPIO
const int ledPin = DATA_PIN;
// Stores LED state
String ledState = "ON";

// Initial colour of the LEDs 'Solid' state
long baseColor = CRGB::Purple;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
// TODO: replace this with REST endpoint like the other state changes
// and update ../data/index.html file to send a GET response rather
// than navigating to a new URL
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}


// Setup an Array for patterns
#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))



// ############## Patterns ##############
// And supporting functions

void rainbow() 
{
  // FastLED's built-in rainbow generator
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti() 
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16( 13, 0, NUM_LEDS-1 );
  leds[pos] += CHSV( gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
}

void solid() {
  for(int i = 0; i < NUM_LEDS; i++ )
   {
     leds[i] = baseColor;
  }
  FastLED.show();
}

// TODO: update to work
// currently only fades to black and then 'jumps'
// back up to max brightness with no fade
// suspect need to remove the brightness check and reassignmen
// out to the bottom of the main loop as with hue and nextPattern
void pulse() {
  for(int i = 0; i < NUM_LEDS; i++ )
   {
   leds[i] = baseColor;
   leds[i].fadeLightBy(BRIGHTNESS);
  }
  FastLED.show();
  EVERY_N_MILLISECONDS( 20 ) {
    BRIGHTNESS = BRIGHTNESS + pulseFadeAmount;
  
  }
  // reverse the direction of the fading at the ends of the fade:
  if(BRIGHTNESS == 0 || BRIGHTNESS == MAX_BRIGHTNESS)
  {
    pulseFadeAmount = -pulseFadeAmount ;
  }
  
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16( i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}

void changeColor(String colorString) {

  baseColor = strtol(colorString.c_str(), NULL, 16);

  // for( int i = 0; i < NUM_LEDS; i++) { //9948
  //   leds[i] = baseColor;
  // }
}

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { rainbow, confetti, sinelon, juggle, bpm };

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}



// ############## Setup ##############
// This will run onece and set up the
// LEDs and
// Listeners for the REST endpoints
void setup(){

  delay(3000); // 3 second delay for recovery
  
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(MAX_BRIGHTNESS);

  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTAGE, MAX_MILLI_AMPS);


  // Serial port for debugging purposes
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);

  // TODO: set up saving state to EPROM/SPIFFS
  
  
  // Initialize SPIFFS
  // Checking the SPIFFS file system is available
  // This is where the public files are stored
  if(!SPIFFS.begin()){ 
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi
  WiFi.softAP(ssid);

  Serial.println("Setting AP (Access Point)…");

  // Print Local IP Address
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH (Turn LEDs ON)
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW (Turn LEDs OFF)
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to change base color (For 'solid' pattern)
  server.on("/changeColor", HTTP_GET, [](AsyncWebServerRequest *request){

    String color = "";
    if (request->hasParam("color")) {
        color = request->getParam("color")->value().c_str();
        changeColor(color);
    } else {
        color = "No color sent";
    }
    request->send(200, "text/plain", "Color changed to " + color);
  });

  // Route to change the pattern
  server.on("/changePattern", HTTP_GET, [](AsyncWebServerRequest *request){

    String pattern = "";
    if (request->hasParam("pattern")) {
        pattern = request->getParam("pattern")->value().c_str();
        currentPattern = pattern;
    } else {
        pattern = "No pattern sent";
    }
    request->send(200, "text/plain", "Pattern changed to " + pattern);
  });

  // Route to change the brightness
  server.on("/changeBrightness", HTTP_GET, [](AsyncWebServerRequest *request){

    String brightness = "";
    if (request->hasParam("brightness")) {
        brightness = request->getParam("brightness")->value().c_str();
        MAX_BRIGHTNESS = brightness.toInt();
        FastLED.setBrightness(MAX_BRIGHTNESS);
    } else {
        brightness = "No brightness sent";
    }
    request->send(200, "text/plain", "brightness changed to " + brightness);
  });

  // Route to change the number of LEDs
  server.on("/changeLedCount", HTTP_GET, [](AsyncWebServerRequest *request){

    String ledCount = "";
    if (request->hasParam("ledCount")) {
        ledCount = request->getParam("ledCount")->value().c_str();
        NUM_LEDS = ledCount.toInt();
    } else {
        ledCount = "No count sent";
    }
    request->send(200, "text/plain", "led count changed to " + ledCount);
  });

  // Start server
  server.begin();
}
 

// ############## Program Loop ##############
void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  if (ledState == "ON") {
    /* code */
    // gPatterns[gCurrentPatternNumber]();

    if(currentPattern == "solid") {
      solid();
    }
    else if(currentPattern == "pulse") {
      pulse();
    }
    else if(currentPattern == "confetti") {
      confetti();
    }
    else if(currentPattern == "rainbow") {
      rainbow();
    }
    else if(currentPattern == "cycle") {
      gPatterns[gCurrentPatternNumber]();
      
    }
    else {
      FastLED.clear();
    }
    // send the 'leds' array out to the actual LED strip
    FastLED.show();  
    // insert a delay to keep the framerate modest
    FastLED.delay(1000/FRAMES_PER_SECOND); 

    // slowly cycle the "base color" through the rainbow
    // for the 'rainbow' pattern
    EVERY_N_MILLISECONDS( 20 ) { gHue++; }
    
    // change the pattern
    // for the 'cycle' pattern
    EVERY_N_SECONDS( 10 ) { nextPattern(); } // change patterns periodically

  } else {
    FastLED.clear(true);
  }
  
}
