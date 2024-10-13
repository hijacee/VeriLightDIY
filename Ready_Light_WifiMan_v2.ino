/*
  DIY VeriLight / ReadyLight for FinishLynx
  by Christoph, 2024
  
  Saves data in JSON file on ESP32
  Uses SPIFFS
  
  The Wifi-Manager functions are based on Bills codes at DroneBot Workshop
  https://dronebotworkshop.com/wifimanager/
 
  Functions based upon sketch by Brian Lough
  https://github.com/witnessmenow/ESP32-WiFi-Manager-Examples
*/
#define FASTLED_INTERNAL
 
// Include Libraries
 
#include <WiFi.h>         // WiFi Library
#include <FS.h>           // File System Library
#include <SPIFFS.h>       // SPI Flash Syetem Library
#include <WiFiManager.h>  // WiFiManager Library
#include <ArduinoJson.h>  // Arduino JSON library
#include <FastLED.h>      // LED Matrix Library
#include <Wire.h>         // 2 Wire communication
#include <Adafruit_GFX.h> // Library for Screen
#include <Adafruit_SSD1306.h> // Library for OLED Screen

#define ESP_DRD_USE_SPIFFS true
#define DATA_PIN 12                             // Define LED-Matrix Data Pin
#define TRIGGER_PIN 13                          // Define Wifi-Reset PIN
#define NUM_LEDS 128                            // Define LED-Matrix Data Pin
#define JSON_CONFIG_FILE "/test_config.json"    // JSON configuration file
#define SCREEN_WIDTH 128                        // OLED display width, in pixels
#define SCREEN_HEIGHT 64                        // OLED display height, in pixels
 
// Flag for saving data
bool shouldSaveConfig = false;
 
// Variables to hold data from custom textboxes
char testString[50] = "test value";
int LynxPort = 10000;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
 

WiFiManager wm;         // Define WiFiManager Object

CRGB leds[NUM_LEDS];    // Define array of LEDs. One item for each LED

WiFiServer server(LynxPort);

const int LED = 14 ;  // Pin der LED
int sizeMsg = 0;
int timeout = 120;    // seconds for AccessPoint to run for after Button Press

// Change to true when testing to force configuration every time we run
bool forceConfig = false;

//timing related variables
unsigned long previousMillis = 0;
const long interval = 1000;  // interval at which to blink (milliseconds)

int colbyte = 0;
int colbytestor = 0;
int blinkbyte = 0;

// Function to light any color depending on 7th bit
void lighton(int a)
{
  for(int i=0;i < NUM_LEDS; i++)
  {
    switch(a) {
      case 0: leds[i] = CRGB::Black;break;
      case 1: leds[i] = CRGB::Red;break;
      case 2: leds[i] = CRGB::Gold;break;        // Instead of "Amber"
      case 3: leds[i] = CRGB::GreenYellow;break; // Instead of "Lemon" 
      case 4: leds[i] = CRGB::Green;break;
      case 5: leds[i] = CRGB::SkyBlue;break;
      case 6: leds[i] = CRGB::Blue;break;
      case 7: leds[i] = CRGB::Purple;break;
      case 8: leds[i] = CRGB::Pink;break;
      case 9: leds[i] = CRGB::White;break;
      default: leds[i] = CRGB::Black;break;
    }
  }

  FastLED.show();

}

//Function to light Matrix yellow middle
void yellowblink() {
  for(int i=0;i < NUM_LEDS; i = i+2)
  {
    leds[i] = CRGB::Yellow;
  }

  FastLED.show();
  delay(1000);
  FastLED.clear();
  FastLED.show();
  delay(1000);
}
 
void saveConfigFile()
// Save Config in JSON format
{
  Serial.println(F("Saving configuration..."));
  
  // Create a JSON document
  JsonDocument json;
  json["testString"] = testString;
  json["LynxPort"] = LynxPort;
 
  // Open config file
  File configFile = SPIFFS.open(JSON_CONFIG_FILE, "w");
  if (!configFile)
  {
    // Error, file did not open
    Serial.println("failed to open config file for writing");
  }
 
  // Serialize JSON data to write to file
  serializeJsonPretty(json, Serial);
  if (serializeJson(json, configFile) == 0)
  {
    // Error writing file
    Serial.println(F("Failed to write to file"));
  }
  // Close file
  configFile.close();
}
 
bool loadConfigFile()
// Load existing configuration file
{
  // Uncomment if we need to format filesystem
  // SPIFFS.format();
 
  // Read configuration from FS json
  Serial.println("Mounting File System...");
 
  // May need to make it begin(true) first time you are using SPIFFS
  if (SPIFFS.begin(false) || SPIFFS.begin(true))
  {
    Serial.println("mounted file system");
    if (SPIFFS.exists(JSON_CONFIG_FILE))
    {
      // The file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open(JSON_CONFIG_FILE, "r");
      if (configFile)
      {
        Serial.println("Opened configuration file");
        JsonDocument json;
        DeserializationError error = deserializeJson(json, configFile);
        serializeJsonPretty(json, Serial);
        if (!error)
        {
          Serial.println("Parsing JSON");
 
          strcpy(testString, json["testString"]);
          LynxPort = json["LynxPort"].as<int>();
 
          return true;
        }
        else
        {
          // Error loading JSON data
          Serial.println("Failed to load json config");
        }
      }
    }
  }
  else
  {
    // Error mounting file system
    Serial.println("Failed to mount FS");
  }
 
  return false;
}
 
 
void saveConfigCallback()
// Callback notifying us of the need to save configuration
{
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
 
void configModeCallback(WiFiManager *myWiFiManager)
// Called when config mode launched
{
  Serial.println("Entered Configuration Mode");
 
  Serial.print("Config SSID: ");
  Serial.println(myWiFiManager->getConfigPortalSSID());
 
  Serial.print("Config IP Address: ");
  Serial.println(WiFi.softAPIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("--------------------");
  display.println("Entered Config. Mode");
  display.println("Config SSID: ");
  display.println(myWiFiManager->getConfigPortalSSID());
  display.println("Config IP: ");
  display.println(WiFi.softAPIP());
  display.display();
}

void setBreakAfterConfig(boolean shouldBreak);
 
void setup()
{
  pinMode(LED,OUTPUT);
  digitalWrite(LED, LOW);
  pinMode(TRIGGER_PIN, INPUT_PULLUP);

  wm.setConfigPortalTimeout(timeout);

  // Setup Serial monitor
  Serial.begin(115200);
  delay(10);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0xBC)) { // Address 0x3D for 128x64 or 0xBC
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("    VeriLight DIY   ");
  display.println("--------------------");
  display.println("Check Configuration");
  display.println("Trigger for next");
  display.println("3 sec");
  display.display();
   
   Serial.println("Check Configuration Trigger for next 3 sec");
         int runtime = millis();
         int starttime = runtime;
         while ((runtime - starttime) < 3000)
         {
             if (digitalRead(TRIGGER_PIN) == LOW)
             {
                 Serial.println("Configuration manually triggered.");

                 display.clearDisplay();
                 display.setTextSize(1);
                 display.setTextColor(WHITE);
                 display.setCursor(0, 0);
                 display.println("    VeriLight DIY   ");
                 display.println("--------------------");
                 display.println("Config. triggered.");
                 display.println("Entering AP-Mode.");
                 display.display();
                   forceConfig = true;
             }
             Serial.print(".");
             display.print(".");
             display.display();
             delay(500);
             runtime = millis();
         }
 
  bool spiffsSetup = loadConfigFile();
  if (!spiffsSetup)
  {
    Serial.println(F("Forcing config mode as there is no saved config"));
    forceConfig = true;
  }
 
  // Explicitly set WiFi mode
  WiFi.mode(WIFI_STA);
 
  // Reset settings (only for development)
  //wm.resetSettings();
 
  // Set config save notify callback
  wm.setSaveConfigCallback(saveConfigCallback);
 
  // Set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wm.setAPCallback(configModeCallback);

  //exit after config instead of connecting
  //wm.setBreakAfterConfig(true);
 
  // Custom elements
 
  // Text box (String) - 50 characters maximum
  WiFiManagerParameter custom_text_box("key_text", "Enter your string here", testString, 50);
  
  // Need to convert numerical input to string to display the default value.
  char convertedValue[6];
  sprintf(convertedValue, "%d", LynxPort); 
  
  // Text box (Number) - 5 characters maximum
  WiFiManagerParameter custom_text_box_num("key_num", "Communication Port", convertedValue, 5); 
 
  // Add all defined parameters
  wm.addParameter(&custom_text_box);
  wm.addParameter(&custom_text_box_num);
 
  if (forceConfig)
    // Run if we need a configuration
  {
    if (!wm.startConfigPortal("ReadyLight_AP", "12345678"))
    {
      Serial.println("failed to connect and hit timeout");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("    VeriLight DIY   ");
      display.println("--------------------");
      display.println("failed to connect");
      display.println("and hit timeout");
      display.println("----- RESTART -----");
      display.display();
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }
  }
  else
  {
    if (!wm.autoConnect("ReadyLight_AP", "12345678"))
    {
      Serial.println("failed to connect and hit timeout");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("    VeriLight DIY   ");
      display.println("--------------------");
      display.println("failed to connect");
      display.println("and hit timeout");
      display.println("----- RESTART -----");
      display.display();
      delay(3000);
      // if we still have not connected restart and try all over again
      ESP.restart();
      delay(5000);
    }
  }
 
  // If we get here, we are connected to the WiFi
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("    VeriLight DIY   ");
  display.println("--------------------");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.print("Port: ");
  display.println(LynxPort);
  display.display();
  
  server.begin(LynxPort);
 
  // Lets deal with the user config values
 
  // Copy the string value
  strncpy(testString, custom_text_box.getValue(), sizeof(testString));
  Serial.print("testString: ");
  Serial.println(testString);
 
  //Convert the number value
  LynxPort = atoi(custom_text_box_num.getValue());
  Serial.print("LynxPort: ");
  Serial.println(LynxPort);
 
 
  // Save the custom parameters to FS
  if (shouldSaveConfig)
  {
    saveConfigFile();
    ESP.restart();
  }

FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // Initialize LED Matrix - GRB ordering is typical

}
 
 
 
 
void loop() {

   WiFiClient client = server.available();    // Listen for incoming clients
   yellowblink();

    if (client) {                             // If a new client connects,
      if (client.connected()) {
        Serial.println("Client connected.");  // print a message out in the serial port
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("    VeriLight DIY   ");
        display.println("--------------------");
        display.print("IP: ");
        display.println(WiFi.localIP());
        display.print("Port: ");
        display.println(LynxPort);
        display.println("--------------------");
        display.println("  CLIENT CONNECTED  ");
        display.display();
        FastLED.clear();
        FastLED.show();
      }
      while (true) {                          // loop while the client's connected   
        if (client.available()) 
        {
          delay(10);                            // delay to allow all bytes to be transferred
          sizeMsg = client.available();         // number of bytes transferred
          char charArray[sizeMsg];            // create array with the size of bytes (-1 because array starts with 0) --> That makes no sense, as the size does not get counted
         
          Serial.print("I received: ");
          Serial.print(sizeMsg);
          Serial.println(" bytes.");
      
          for (int i=0; i < sizeMsg; i++)     // for-loop to fill the array wiht the transferred bytes
            {
              charArray[i] = client.read();     // fill the array
        
              Serial.print("Byte ");
              Serial.print(i+1);
              Serial.print(": ");
              Serial.println(charArray[i], HEX);  //print out the content of the array at i
           }
           if (sizeMsg > 7)
           {
            colbyte = charArray[6];                   // convert 7th byte into int
            colbytestor = charArray[6];               // create a colbytestorage for blinking
            Serial.print("The 7th bit is: ");
            Serial.print(colbyte);
            if (sizeMsg > 12)
            {
              blinkbyte = charArray[12];
              Serial.print("The blinkbit is: ");
              Serial.print(blinkbyte);
            }
           }
           else
           {
            Serial.println("Msg smaller than 7 byte - LED off."); //Wenn Initialisiert wird, die Zeit läuft oder pausiert ist (weniger als 7 bytes), keine Freigabe; d.h. ABC000
            digitalWrite(LED, LOW);
            colbyte = 0;
           }
           
        }
        unsigned long currentMillis = millis();
          
        if (blinkbyte == 1)               // check if the blinkbyte is 1
        {
          if (currentMillis - previousMillis >= interval)
          {
            previousMillis = currentMillis;
            if (colbyte == colbytestor)
            {
              colbyte = 0;
            }
            else
            {
              colbyte = colbytestor;
            }
              lighton(colbyte);
           }
        }
         else
         {
           lighton(colbyte);                         // call lighton function, color dependend on 7th byte, blinking dependend on 13th byte (off by default)
         }
          
        
        
      if (!client.connected())                        // if the client disconnects
      {
        Serial.println("Client disconnected.");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("    VeriLight DIY   ");
        display.println("--------------------");
        display.print("IP: ");
        display.println(WiFi.localIP());
        display.print("Port: ");
        display.println(LynxPort);
        display.println("--------------------");
        display.println(" CLIENT DISCONNECTED");
        display.display();
        client.stop();                            // stop the client funtion
        digitalWrite(LED, LOW);                   // turn out the LED
        FastLED.clear();
        FastLED.show();
        break;                                    // leave the for loop to be ready for new connection 
      }
     }  
    }    
 
}
