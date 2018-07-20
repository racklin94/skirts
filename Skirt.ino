#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_FloraPixel.h>

#define PIN 12
#define NUM_PIXEL 20
#define compass 1
#define twinkle 2

unsigned long AccelMillis;
unsigned long MagMillis;
unsigned long ModeMillis;
int mode = compass;
sensors_event_t Accevent;
sensors_event_t Magevent;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXEL, PIN, NEO_GRB + NEO_KHZ800);

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified Accel = Adafruit_LSM303_Accel_Unified(54321);
Adafruit_LSM303_Mag_Unified Mag = Adafruit_LSM303_Mag_Unified(12345);


void displaySensorDetails(void)
{
  sensor_t sensor;
  Mag.getSensor(&sensor);
  Accel.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Accelerometer Test"); Serial.println("");

    /* Enable auto-gain */
  Accel.enableAutoRange(true);
  Mag.enableAutoRange(true);
  
  strip.begin();
  // Update the strip, to start they are all 'off'
  strip.show();
  /* Initialise the sensor */
  if(!Mag.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
   if(!Accel.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
  strip.setBrightness(128 );
}

void loop() {
  // put your main code here, to run repeatedly:
  /* Get a new sensor event */ 
  unsigned long currentmillis = millis();
  Accel.getEvent(&Accevent);
  Mag.getEvent(&Magevent);

 //   //define movement and change the mode of the lights
    if (Accevent.acceleration.x>=15){
       Serial.print("Accel X: "); Serial.print(Accevent.acceleration.x); Serial.print("\n");
       //on jump
      //;
      mode = twinkle;
    }
    else{
      mode = compass;
    }
  LED_modes(mode);

}

//*********************************************************************************************
//*************************         LED_modes         *****************************************
//*********************************************************************************************

void LED_modes(int local_mode){
  uint8_t theaterWait = 0;
  float headingLocal;
  unsigned long lCurrentMillis = millis();
  uint32_t lColor;
  uint32_t red = strip.Color(255, 0, 0);
  uint32_t white = strip.Color(255, 255, 255);

  
  switch (local_mode){
    case compass:
        Serial.println("Entering Compass Mode");

        lColor = compass_heading();


      Serial.print("Current Millis:  ");
      Serial.print(lCurrentMillis);
      Serial.println("");
      Serial.print("Mode Millis:  ");
      Serial.print(ModeMillis);
      Serial.println("");

      if((lCurrentMillis>=0)&&(lCurrentMillis<=ModeMillis+30000)){
        Serial.println("First Mode");
        colorWipe(lColor,50);
      } else if((lCurrentMillis > ModeMillis+30001) && (lCurrentMillis<=ModeMillis+60000)){
        Serial.println("Second Mode");
        theaterWait = random(0, 75);
        theaterChase(lColor, 50);
      } else if((lCurrentMillis > ModeMillis+60001) && (lCurrentMillis<=ModeMillis+90000)){
        Serial.println("Third Mode");
        for(int x =0; x<25;x++){
          Mag.getEvent(&Magevent);
          lColor = compass_heading();
          Twinkle_Color(lColor, 20);
        }
      } else if((lCurrentMillis > ModeMillis+90001) && (lCurrentMillis<=ModeMillis+120000)){
        Serial.println("Fourth Mode");
          rainbowCycle(20,3);
          ModeMillis = lCurrentMillis;
      } else if(lCurrentMillis > ModeMillis+120001){
          ModeMillis = lCurrentMillis;
      } else if(lCurrentMillis < ModeMillis){
          ModeMillis = lCurrentMillis;
          AccelMillis = lCurrentMillis;
          MagMillis = lCurrentMillis;
      }
      break;
    case twinkle:
      Serial.println("Entering Twinkle Mode on Jump");
      for(int x =0; x<25;x++){
       Twinkle(20);
      }
      local_mode = compass;
      ModeMillis = lCurrentMillis;
      break;
    default:
      setAllColor(strip.Color(255,0, 0));
      break;
  }
//  return local_mode;
  
}

//*********************************************************************************************
//*************************      compass_heading      *****************************************
//*********************************************************************************************

uint32_t compass_heading(void){

  float Pi = 3.14159;
  uint32_t newColor = 0;

  // Calculate the angle of the vector y,z
  float heading = (atan2(Magevent.magnetic.y,Magevent.magnetic.z)*180)/Pi;

  //Normalize to 0-360
  if(heading < 0){
    heading = 350 + heading;
  }

//  Serial.print("Compass Heading: ");
//  Serial.print(heading);
//  Serial.print("\n");

  newColor = map(heading,0,360,0,16581375);
  return newColor;
}

//*********************************************************************************************
//*************************          Twinkle          *****************************************
//*********************************************************************************************

//maybe make the time delay the delta of acceleration 
void Twinkle(uint32_t max_wait){
  uint32_t brightness = random(0, 255);
  uint32_t pixel = random(0, NUM_PIXEL);

  colorWipe(strip.Color(0,0,0),max_wait);
  strip.show();
  strip.setPixelColor(pixel, strip.Color(brightness,brightness,brightness));
  strip.show();
  delay(max_wait);
  strip.setPixelColor(pixel, strip.Color(0,0,0));
  strip.show();
}

//*********************************************************************************************
//*************************       Twinkle_Color       *****************************************
//*********************************************************************************************

//maybe make the time delay the delta of acceleration 
void Twinkle_Color(uint32_t c, uint32_t max_wait){
  uint32_t pixel = random(0, NUM_PIXEL);

  colorWipe(strip.Color(0,0,0),max_wait);
  strip.show();
  strip.setPixelColor(pixel, c);
  strip.show();
  delay(max_wait);
  strip.setPixelColor(pixel, strip.Color(0,0,0));
  strip.show();
}

//*********************************************************************************************
//*************************        setAllColor        *****************************************
//*********************************************************************************************

void setAllColor(uint32_t c){
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

//*********************************************************************************************
//*************************         colorWipe         *****************************************
//*********************************************************************************************

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

//*********************************************************************************************
//*************************      rainbow_nodelay      *****************************************
//*********************************************************************************************

void rainbow_nodelay(uint8_t wait, uint8_t current_time) {
  static uint16_t local_time,i,j;

  //dont update unless its time to
  if(current_time>local_time+wait){
    local_time=current_time;

    //for each color
    if(j<256){
      //for each pixel
      if(i<strip.numPixels()) {
        //set the color
        strip.setPixelColor(i, Wheel((i+j) & 255));
        i++; //next pixel
      }
      else{
        i=0; //go to beginning pixel
      }
      j++; //inc on color wheel
      strip.show();
    }
    else{
      j=0; //reset to beginning of color wheel
    }
  }
}

//*********************************************************************************************
//*************************        rainbowCycle       *****************************************
//*********************************************************************************************

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait, int numOfIterations) {
  uint16_t i, j;

  for(j=0; j<256*numOfIterations; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}


//*********************************************************************************************
//*************************        theaterChase       *****************************************
//*********************************************************************************************

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//*********************************************************************************************
//*************************    theaterChaseRainbow    *****************************************
//*********************************************************************************************

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (uint16_t i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//*********************************************************************************************
//*************************           Wheel           *****************************************
//*********************************************************************************************

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

void CandyCane  (uint32_t color1, uint32_t color2, int sets,int width,int wait) {
  int L;
  for(int j=0;j<(sets*width);j++) {
    for(int i=0;i< strip.numPixels();i++) {
      L=strip.numPixels()-i-1;
      if ( ((i+j) % (width*2) )<width){
        strip.setPixelColor(L,color1);
      }else{
        strip.setPixelColor(L,color2);
      }
    }
    strip.show();
    delay(wait);
  }
}

