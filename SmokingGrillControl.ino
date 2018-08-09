
#include "SSD1306Wire.h"
#include <driver/adc.h>

#include "images.h"
#include "keys.h"

#include "esp32-hal.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <IOXhop_FirebaseESP32.h>

WiFiMulti wifiMulti;

SSD1306Wire  display(0x3c, 5, 4);

#define ANALOG_PIN_0 25

int demoMode = 0;
int counter = 1;
int adcval = 0;

float temp_celsius;

int dataArray[100];
int currentPointIndex = 0;

void initGraph() {
  display.setColor(WHITE);
  for (int i = 0; i < 100 ; i++)
    dataArray[i] = 0;
}


void setup() {
  Serial.begin(115200);
  Serial.println("Hello");
  Serial.println();


  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(Dialog_plain_7);
  //  pinMode(ANALOG_PIN_0, INPUT);
  adc1_config_width(ADC_WIDTH_12Bit);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  initGraph();
}

void drawImageDemo() {
  // see http://blog.squix.org/2015/05/esp8266-nodemcu-how-to-create-xbm.html
  // on how to create xbm files
  display.drawXbm(34, 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);

}

void updateDataToCloud(int data, float temperture) {
  if ((wifiMulti.run() == WL_CONNECTED)) {
    HTTPClient http;
    Serial.println("adc : " + String(data));
    http.begin("https://api.thingspeak.com/update?api_key="+ String(THINGSPEAK_KEY) +"&field1=" + String(data) + "&field2=" + String(temperture)); //HTTP
    int httpCode = http.GET();
    http.end();
  }
}
void loop() {

  //    Serial.println("Hello");
  // clear the display
  display.clear();

  display.drawHorizontalLine(10, 54, 100);
  display.drawVerticalLine(10, 5, 50);

  //  adcval = analogRead(ANALOG_PIN_0);
  adcval = adc1_get_voltage(ADC1_CHANNEL_0);



  float voltage = adcval * (3.9 / 4095.0);

  //  int voltage =  adc1_get_voltage(ADC1_CHANNEL_0);
  //  temp_celsius = (voltage - 0.5) * 100 ;
  temp_celsius = (adcval * 0.0424) + 6 ;


  if (counter > 15) {
    updateDataToCloud(adcval, temp_celsius);
    counter = 0;
  }

  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  //  display.drawString(64, 50, "Temperture :" + String(temp_celsius));
  //  display.drawString(64, 54, "Voltage :" + String(voltage));
  display.drawString(64, 54, "Temperture :" + String(temp_celsius));
  display.drawString(64, 0, "adc :" + String(adcval));
  // draw the current demo method
  //  drawImageDemo();

  updateGraph(temp_celsius / 4);
  display.display();
  //updateGraph((adcval - 568) / 20);

  counter++;
  delay(1000);
}

void updateGraph(int val) {
  dataArray[currentPointIndex] = val ;

  Serial.println("val : " + String(val) + " currentPoint : " + String(55 - dataArray[currentPointIndex]));

  for (int i = 0 ; i < 100 ; i++) {
    int x = i;
    int y = 0;
    if (100 - i < currentPointIndex) {
      y = dataArray[currentPointIndex - (100 - i)] ;
    } else {
      y = dataArray[currentPointIndex + i];
    }
    display.setPixel(x + 10, 54 - y );
  }
  currentPointIndex++;
  if (currentPointIndex >= 99) {
    currentPointIndex = 0;
  }

}

