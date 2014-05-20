/* Ambiled: Arduino sketch that listens to serial data
 * for RGB values followed by something in order to send data
 * to the LED strip.
 *
 * To use this, you will need to plug an Addressable RGB LED
 * strip from Pololu into pin 12.
 *
 * The format of the color should be "R,G,B!" where R, G, and B
 * are numbers between 0 and 255 representing the brightnesses
 * of the red, green, and blue components respectively.
 *
 * Please note that this sketch only transmits colors to the
 * LED strip after it receives them from the computer, so if
 * the LED strip loses power it will be off until you resend
 * the color.
 */
 
#include <PololuLedStrip.h>
#include <WS2801.h>

#define DATA_PIN 12

#define SETTINGS_BYTES 2

#define WS2801_TYPE  0
#define WS2812B_TYPE 1
#define TM1804_TYPE  2

void setup()
{ 
  // Start up the serial port, for communication with the PC.
  Serial.begin(115200);
  Serial.print("ready\n");

  uint8_t leds = 0;
  uint8_t ledsIc = WS2801_TYPE;
  uint8_t buffer[SETTINGS_BYTES];
  int bytesRead = 0;
  
  for (;;)
  {
    if(Serial.available())
    {
      buffer[bytesRead] = Serial.read();
      if(buffer[bytesRead] != -1)
      {
        bytesRead++;
        if(bytesRead >= SETTINGS_BYTES)
        {
          break;
        }
      }
    }
  }
  
  leds = buffer[0];
  ledsIc = buffer[1];
  
  switch(ledsIc)
  {
    case WS2801_TYPE:
      run_WS2801(leds);
      break;
    case WS2812B_TYPE:
      run_WS2812B(leds);
      break;
//    case TM1804_TYPE:
//      run_TM1804(leds);
    default:
      Serial.print("Invalid led type\n");
      break;
  }
}

void run_WS2801(int leds)
{
  uint8_t dataPin  = 2;
  uint8_t clockPin = 3;
  
  WS2801 strip = WS2801(leds, dataPin, clockPin);
  strip.begin();
  
  Serial.print("ready\n");

  int bytesToRead = leds * 3;  
  uint8_t buffer[bytesToRead];
  int bytesRead = 0;
  
  for(;;)
  {
    if(Serial.available())
    {
      buffer[bytesRead] = Serial.read();
      if(buffer[bytesRead] != -1)
      {
        bytesRead++;
      }
      
      if(bytesRead >= bytesToRead)
      {
        bytesRead = 0;
        Serial.print("ack\n");
        
        for(int i = 0; i < leds; i++)
        {
          strip.setPixelColor(i, buffer[i *3],
                                 buffer[(i * 3) + 1],
                                 buffer[(i * 3) + 2]);
        }
        
        strip.show();
      }
    }
  }
}

void setColor(PololuLedStrip<DATA_PIN> strip, rgb_color color, int leds);
void writeToLeds(PololuLedStrip<DATA_PIN> strip, const uint8_t* buffer, int leds);

void run_WS2812B(int leds)
{
  PololuLedStrip<DATA_PIN> strip;

  Serial.print("ready\n");
  
  int bytesToRead = leds * 3;

  int data;
  uint8_t buffer[bytesToRead];
  rgb_color colors[leds];
  int bytesRead = 0;
  int ledCount = 0;
  
  for(;;)
  {
    if(Serial.available())
    {
      data = Serial.read();
      if(data != -1)
      {
        buffer[bytesRead] = data;
        bytesRead++;
      }
      
      if(bytesRead >= bytesToRead)
      {
        bytesRead = 0;
        writeToLeds(strip, buffer, leds);
        Serial.print("ack\n");
      }
    }
  }
}

void setColor(PololuLedStrip<DATA_PIN> strip, rgb_color color, int leds)
{
  rgb_color colors[leds];
  
  for(uint16_t i = 0; i < leds; i++)
  {
    colors[i] = color;
  }
  strip.write(colors, leds);
}

void writeToLeds(PololuLedStrip<DATA_PIN> strip, const uint8_t* buffer, int leds)
{
  rgb_color color;
  rgb_color colors[leds];
//  rgb_color colors[59];
  for (int i = 0; i < 59; i++) {
    color.red = 0;
    color.green = 0;
    color.blue = 0;
    colors[i] = color;
  }

  for(int i = 0; i < leds; i++)
  {
    color.red = buffer[i * 3];
    color.green = buffer[(i * 3) + 1];
    color.blue = buffer[(i * 3) + 2];

//    int led = i * 3;
//    colors[led] = color;
    colors[i] = color;
  }
  
  strip.write(colors, leds);
//  strip.write(colors, 59);
}

void loop() {}
