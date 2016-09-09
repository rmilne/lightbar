// This is a demonstration on how to use an input device to trigger changes on your neo pixels.
// You should wire a momentary push button to connect from ground to a digital IO pin.  When you
// press the button it will change to a new pixel animation.  Note that you need to press the
// button once to start the first animation!

#include <Adafruit_NeoPixel.h>
#include <avr/eeprom.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define BUTTON_PIN   3    // Digital IO pin connected to the button.  This will be
                          // driven with a pull-up resistor so the switch should
                          // pull the pin to ground momentarily.  On a high -> low
                          // transition the button press logic will execute.

#define PIXEL_PIN    0    // Digital IO pin connected to the NeoPixels.
#define LED 1
#define PIXEL_COUNT 12



// Parameter 1 = number of pixels in strip,  neopixel stick has 8
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream, correct for neopixel stick
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip), correct for neopixel stick
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

bool oldState = LOW;
int colour_state = 0; // 0 is hold colour, 1 is set new colour
uint32_t colour;
uint32_t addr = 1;

void setup() {
    // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  pinMode(LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);

  // init the strip
  strip.begin();
  colorWipe(strip.Color(255,255,255), 50);
  strip.show();
  
  // read saved colour from eeprom
  while (!eeprom_is_ready());
  colour = eeprom_read_dword((uint32_t *)addr);
  colorWipe(colour, 100);
  strip.show();
}

void loop() {
  // Get current button state.
  bool newState = digitalRead(BUTTON_PIN);

  // Check if state changed from high to low (button press).
  if (newState == LOW && oldState == HIGH) {
    // Short delay to debounce button.
    digitalWrite(LED, HIGH); 
    delay(20);
    // Check if button is still low after debounce.
    newState = digitalRead(BUTTON_PIN);
    if (newState == LOW) {
      // button has been pressed
      digitalWrite(LED, LOW);
      colour_state = !colour_state;
      if (colour_state != 0) {
        // run the rainbow until button press
        colour = rainbow_button(30);
        // save colour to eeprom 
        eeprom_write_dword((uint32_t *)addr, colour);
        delay(500);
        colorWipe(strip.Color(0,0,0),10);
        colorWipe(colour, 20);
        colorWipe(strip.Color(0,0,0),10);
        colorWipe(colour, 20);
      }
    }
  }
  // Set the last button state to the old state.
  oldState = newState;
}


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

uint32_t rainbow_button(uint8_t wait) {
  uint16_t i, j;
  bool button = LOW;

  do {
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
      
    for(j=0; j<256; j++) {
      for(i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, Wheel((i+j) & 255));
      }
      strip.show();
      delay(wait);
      button = digitalRead(BUTTON_PIN);
      if (button == LOW) {
        return Wheel(i+j);
      }
    }   
  } while (button == HIGH);

  return Wheel(i+j);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
