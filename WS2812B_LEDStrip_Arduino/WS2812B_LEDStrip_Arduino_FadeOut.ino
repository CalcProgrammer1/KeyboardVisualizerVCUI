/*---------------------------------------------------------*\
|                                                           |
|   WS2812B (NeoPixel) Serial Controlled LED Strip Driver   |
|     for use with Keyboard Visualizer VCUI                 |
|                                                           |
|   Adam Honse (calcprogrammer1) 12/9/2016                  |
|                                                           |
\*---------------------------------------------------------*/
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 10
#define LEDS 54
#define PACKET_SZ ( (LEDS * 3) + 3 )

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.
unsigned char serial_buffer[PACKET_SZ];
unsigned int head = 0;
unsigned int start;
unsigned int checksum_1;
unsigned int checksum_0;
unsigned long previousMillis;

void setup()
{
  Serial.begin(115200);
  strip.begin();
  strip.show(); 
  previousMillis = millis();     
}

void loop()
{
  if( Serial.available())
  {

    serial_buffer[head] = Serial.read();
  
    if( head >= (PACKET_SZ - 1) )
    {
      start = 0;
      checksum_1 = head;
      checksum_0 = head - 1;
      head = 0;
    }
    else
    {
      start = head + 1;
      checksum_1 = head;
      if( head == 0 )
      {
        checksum_0 = PACKET_SZ - 1;        
      }
      else
      {
        checksum_0 = head - 1;        
      }
      head++;
    }
  
    if( serial_buffer[start] == 0xAA )
    {
      unsigned short sum = 0;      
  
      for( int i = 0; i < checksum_0; i++ )
      {
        sum += serial_buffer[i];
      }
  
      if( start > 0 )
      {
        for( int i = start; i < PACKET_SZ; i++ )
        {
          sum += serial_buffer[i];
        }
      }
      
      //Test if valid write packet
      if( ( ( (unsigned short)serial_buffer[checksum_0] << 8 ) | serial_buffer[checksum_1] ) == sum )
      {
        noInterrupts();
        for( int i = 0; i < LEDS; i++ )
        {
          int idx = start + 1 + ( 3 * i );
    
          if( idx >= (PACKET_SZ - 1) )
          {
            idx = idx - PACKET_SZ;
          }
          
          strip.setPixelColor(i, strip.Color(serial_buffer[idx], serial_buffer[idx+1], serial_buffer[idx+2]));
          
        }
        previousMillis = millis();
        strip.show();
        interrupts();
    
      }
    }
  }
  if ((millis() - previousMillis) > 1000)
  {
    bool black = false;
    uint32_t colorBlack = strip.Color(0,0,0);
    while (!black) 
    {
      for( int i = 0; i < LEDS; i++ ) 
      {
      if ( colorBlack != strip.getPixelColor(i)) 
        { 
          uint32_t c = strip.getPixelColor(i);
          int r = (uint8_t)(c >> 16);
          int g = (uint8_t)(c >>  8);
          int b = (uint8_t)c;
          strip.setPixelColor(i, strip.Color(r*0.8,g*0.8,b*0.8));
        } 
        else 
        {
          black = true;   
        }  
      }
      strip.show(); 
    }
    previousMillis=millis(); 
  } 
}
    
  

      
