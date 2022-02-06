#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define NEO 4
#define MAGNET 5
#define LDR A0

#define S2 6
#define S3 9
#define sensorOut 2
#define powerPin 3

#define ID 1 // 1, 2 oder 3

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00011";

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, NEO, NEO_RGBW + NEO_KHZ800);



int red = 255;
int green = 255;
int blue = 0;

int LDRthreshold = 300;

boolean isActive = true;
boolean lastStatus = false;

long lastWake = 0;
long blinkSignal1 = 0;
long blinkSignal2 = 0;


// ******


void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  strip.begin();


  for (int i = 0; i < 6; i++) {
    strip.setPixelColor(i, strip.Color(red, green, blue));
  }
  strip.show();


  pinMode(MAGNET, INPUT);
  digitalWrite(MAGNET, HIGH);
  pinMode(LDR, INPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, LOW); // Ich schalte das aus fuer diese einfache Version

  if (ID == 1) {
    LDRthreshold = 150;
  } 
  if (ID == 2) {
    LDRthreshold = 230;
  } 
  if (ID == 3) {
    LDRthreshold = 360;
  } 

  lastWake = millis();
}

void loop() {
  delay(50);
  Serial.println(analogRead(LDR));
  //Serial.println(digitalRead(MAGNET));
  if (analogRead(LDR) > LDRthreshold) {
    showPixels(255, 255, 255, wasActivated());
    //digitalWrite(powerPin, LOW);
  } else {
    showPixels(red, green, blue, wasActivated());
    digitalWrite(powerPin, LOW);
  }


  if (radio.available()) {
    lastWake = millis();
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);

    if (strcmp(text, "NDU_RESET") == 0) {
      red = 255;
      green = 255;
      blue = 0;
    }
    if (strcmp(text, "NDU_RAIN") == 0) {
      rainbowCycle(20);
    }

    if (strcmp(text, "NDU_SELECT1") == 0) {
      if (ID == 1) {
        isActive = true;
        lastStatus = false;
        // Blink-Animation
      } else {
        isActive = false;
        lastStatus = false;
        Serial.println("not active");
      }
    }

    if (strcmp(text, "NDU_SELECT2") == 0) {
      if (ID == 2) {
        isActive = true;
        lastStatus = false;
        // Blink-Animation
      } else {
        isActive = false;
        lastStatus = false;
      }
    }

    if (strcmp(text, "NDU_SELECT3") == 0) {
      if (ID == 3) {
        isActive = true;
        lastStatus = false;
        // Blink-Animation
      } else {
        isActive = false;
        lastStatus = false;
      }
    }


    if (strcmp(text, "NDU_RANDOM") == 0) {
      //if (isActive) {
        red = random(0, 255);
        green = random(0, 255);
        blue = random(0, 255);
      //}
    }

    
    if (strcmp(text, "NDU_RPLUS") == 0) {
      if (isActive) {
        red = red + 56;
        if (red >=255) 
         red = 255;
      }
    }

    if (strcmp(text, "NDU_RMINUS") == 0) {
      if (isActive) {
        red = red - 56;
        if (red <= 0) 
         red = 0;
      }
    }

    if (strcmp(text, "NDU_GPLUS") == 0) {
      if (isActive) {
        green = green + 56;
        if (green >=255) 
         green = 255;
      }
    }

    if (strcmp(text, "NDU_GMINUS") == 0) {
      if (isActive) {
        green = green - 56;
        if (green <= 0) 
         green = 0;
      }
    }    

    if (strcmp(text, "NDU_BPLUS") == 0) {
      if (isActive) {
        blue = blue + 56;
        if (blue >=255) 
         blue = 255;
      }
    }

    if (strcmp(text, "NDU_BMINUS") == 0) {
      if (isActive) {
        blue = blue - 56;
        if (blue <= 0) 
         blue = 0;
      }
    }

  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, c);  //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j = 0; j < 256; j++) {   // cycle all 256 colors in the wheel
    for (int q = 0; q < 3; q++) {
      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i = 0; i < strip.numPixels(); i = i + 3) {
        strip.setPixelColor(i + q, 0);      //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



void showPixels(int r_, int g_, int b_, boolean blink_) {
  if (blink_ == true) {
    blinkSignal1 = millis();
    blink_ = false;
  }

  
  if (abs(millis() - lastWake) < 15000) {   
  } else {
    r_ = 0; g_ = 0; b_ = 0;
  }

  if ((abs(millis() - blinkSignal1) < 150)) {   
    r_ = 0; g_ = 0; b_ = 0;
  } 

  if ((abs(millis() - blinkSignal1) > 300) && (abs(millis() - blinkSignal1) < 450)) {   
    r_ = 0; g_ = 0; b_ = 0;
  }   

  if ((abs(millis() - blinkSignal1) > 600) && (abs(millis() - blinkSignal1) < 750)) {   
    r_ = 0; g_ = 0; b_ = 0;
  }   
  
  for (int i = 0; i < 6; i++) {
    strip.setPixelColor(i, strip.Color(r_, g_, b_));
    strip.show();
  }
  
}


boolean wasActivated() {
  if (isActive == lastStatus) {
    return false;
  } else {
    lastStatus = true;
    return true;
  }
}
