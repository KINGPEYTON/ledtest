// Simple strand test for Adafruit Dot Star RGB LED strip.
// This is a basic diagnostic tool, NOT a graphics demo...helps confirm
// correct wiring and tests each pixel's ability to display red, green
// and blue and to forward data down the line.  By limiting the number
// and color of LEDs, it's reasonably safe to power a couple meters off
// the Arduino's 5V pin.  DON'T try that with other code!

#include <Adafruit_DotStar.h>
#include <bluefruit.h>

// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

\
#define NUMPIXELS 72 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN    7 //green
#define CLOCKPIN   9 //yellow
Adafruit_DotStar strip1(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Here's how to control the LEDs from any two pins:
#define DATAPIN2    10 //green
#define CLOCKPIN2   11 //yellow
Adafruit_DotStar strip2(NUMPIXELS, DATAPIN2, CLOCKPIN2, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip(NUMPIXELS, DOTSTAR_BRG);

// BLE Service
BLEDfu  bledfu;
BLEDis  bledis;
BLEUart bleuart;

#define numModes    5 //how many modes there are

void setup() {

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  Serial.begin(115200);
 // while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Adafruit Bluefruit Neopixel Test");
  Serial.println("--------------------------------");

  Serial.println();
  Serial.println("Please connect using the Bluefruit Connect LE application");
  
  // Init Bluefruit
  Bluefruit.begin();
  Bluefruit.setName("Dicks For Harambe");
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  Bluefruit.Periph.setConnectCallback(connect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();  

  // Configure and start BLE UART service
  bleuart.begin();

  // Set up and start advertising
  startAdv();

  beginStrip(&strip1);
}

int      head  = 0, tail = -10; // Index of first 'on' and 'off' pixels
uint32_t color = 0xFF0000;      // 'On' color (starts red)
uint32_t colors[] = {0x8a1a1a, 0x32a838, 0x0011d1, 0xfbff00, 0xff00fb, 0x00f7ff};

bool on = true;
int mode1 = 0;
int mode2 = 0;

int prevCom;
bool buttonUp = true;

void loop() { 
  if ( Bluefruit.connected() && bleuart.notifyEnabled())
  {
    int command = bleuart.read();
    if(command > 1){
      mode1 = changeLedMode(mode1, command);
      Serial.print((char) command);
    }
  }

  if(on){
    setLEDMode(mode1, &strip1);
  }
}

//
void setLEDMode(int mode, Adafruit_DotStar *strip){
  switch (mode) {
      case 0: {   // Get Version
          stripMode(strip);
          break;
        }
      case 1: {   // Get Version
          raveMode(strip);
          break;
        }
      case 2: {   // Get Version
          randomChaos(strip);
          break;
        }
      case 3: {   // Get Version
          meteorRainGradient(strip, colors[random(0, 6)], 10, 32, true, random(5,60), random(10,70));
          break;
        }
      case 4: {   // Get Version
          meteorRain(strip, 0x6c,0x38,0xc7,10, 64, true, 30);
          break;
        }
      case 5: {   // Get Version
          BouncingBalls(strip, 0xff,0,0, 3);
          break;
        }
      case 6: {   // Get Version
          meteorRain(strip, 0x6c,0x38,0xc7,10, 64, true, 30);
          break;
        }
  }
}

int changeLedMode(int mode, int command){
  //Serial.println((char) command);
  if (on) {
    if(command == '7'){
      if(buttonUp){
        mode--;
        if(mode < 0){
          mode = numModes;
        }
        buttonUp = false;
      } else {
        buttonUp = true;
      }
    } else if (command == '8') {
      if(buttonUp){
        mode++;
        if(mode > numModes){
          mode = 0;
        }
          buttonUp = false;
        } else {
          buttonUp = true;
        }
    }
  } else if (command == '5' && prevCom == 'B') {
    if(buttonUp){
      if (on) {
        setAll(&strip1, 0,0,0); // Set all led to off
        on = false;
      } else {
        on = true;
      }
      
      buttonUp = false;
    } else {
      buttonUp = true;
    }
  }
  prevCom = command;
  return mode;
}

//modes 
void stripMode(Adafruit_DotStar *strip){
  strip->setPixelColor(head, color); // 'On' pixel at head
  strip->setPixelColor(tail, 0);     // 'Off' pixel at tail
  strip->show();                     // Refresh strip
  delay(20);                        // Pause 20 milliseconds (~50 FPS)

  if(++head >= NUMPIXELS) {         // Increment head index.  Off end of strip?
    head = 0;                       //  Yes, reset head index to start
    if((color >>= 8) == 0)          //  Next color (R->G->B) ... past blue now?
      color = 0xFF0000;             //   Yes, reset to red
  }
  if(++tail >= NUMPIXELS) tail = 0; // Increment, reset tail index
  
}

void randomChaos(Adafruit_DotStar *strip){
  int randPix = random(NUMPIXELS);
  int randColor = random(16777216);

  strip->setPixelColor(randPix, strip->gamma32(strip->ColorHSV(randColor)));     // 'Off' pixel at tail
  strip->show();
  
  delay(2);                        // Pause 20 milliseconds (~50 FPS)
}

void raveMode(Adafruit_DotStar *strip){
    //strip->setBrightness(64);
    uint32_t col = colors[random(0, 6)];
    for(int i = 0; i < NUMPIXELS; i++){
      strip->setPixelColor(i, col); // 'On' pixel at head
    }  
    strip->show();                     // Refresh strip
    delay(300);
}

void meteorRainGradient(Adafruit_DotStar *strip, uint16_t startColor, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay, int colorSpeed) {  
  //setAll(strip, 0,0,0);
  uint16_t mc = startColor;
  
  for(int i = 0; i < NUMPIXELS+NUMPIXELS; i++) {

    if(bleuart.read() == 'B'){
      break;
    }
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUMPIXELS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(strip, j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUMPIXELS) && (i-j>=0) ) {
        strip->setPixelColor(i-j,strip->gamma32(strip->ColorHSV(mc,255,255)));
        mc += colorSpeed;
        //setPixel(i-j, red, green, blue);
      } 
    }
   
    strip->show();
    delay(SpeedDelay);
  }
}

void meteorRain(Adafruit_DotStar *strip, byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  //setAll(strip, 0,0,0);
  
  for(int i = 0; i < NUMPIXELS+NUMPIXELS; i++) {

    if(bleuart.read() == 'B'){
      break;
    }
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUMPIXELS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(strip, j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUMPIXELS) && (i-j>=0) ) {
        setPixel(strip, i-j, red, green, blue);
        //setPixel(i-j, red, green, blue);
      } 
    }

    strip->show();
    delay(SpeedDelay);
  }
}

void BouncingBalls(Adafruit_DotStar *strip, byte red, byte green, byte blue, int BallCount) {
  float Gravity = -9.81;
  int StartHeight = 1;
  
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
  
  for (int i = 0 ; i < BallCount ; i++) {
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0; 
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2); 
  }

  while (true) {
    if(bleuart.read() == 'B'){
      break;
    }
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
  
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
  
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUMPIXELS - 1) / StartHeight);
    }
  
    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(strip, Position[i],red,green,blue);
    }
    
    strip->show();
    setAll(strip, 0,0,0);
  }
}

void BouncingColoredBalls(Adafruit_DotStar *strip, int BallCount, byte colors[][3]) {
  float Gravity = -9.81;
  int StartHeight = 1;
  
  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];
  
  for (int i = 0 ; i < BallCount ; i++) {   
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0; 
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2); 
  }

  while (true) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;
  
      if ( Height[i] < 0 ) {                      
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();
  
        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUMPIXELS - 1) / StartHeight);
    }
  
    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(strip, Position[i],colors[i][0],colors[i][1],colors[i][2]);
    }
    
    strip->show();
    setAll(strip, 0,0,0);
  }
}

void Fire(Adafruit_DotStar *strip, int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUMPIXELS];
  int cooldown;
  
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUMPIXELS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUMPIXELS) + 2);
    
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
  
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUMPIXELS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
    
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUMPIXELS; j++) {
    setPixelHeatColor(strip, j, heat[j] );
  }

  strip->show();
  delay(SpeedDelay);
}

//mode helper methods
void fadeToBlack(Adafruit_DotStar *strip, int ledNo, byte fadeValue) {
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
    
    oldColor = strip->getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
    
    strip->setPixelColor(ledNo, r,g,b);

}

void setPixelHeatColor (Adafruit_DotStar *strip, int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);
 
  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(strip, Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(strip, Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(strip, Pixel, heatramp, 0, 0);
  }
}

//bluetooth methods
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);

  Serial.println("Please select the 'Neopixels' tab, click 'Connect' and have fun");
}

void startAdv(void)
{  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

// strip help methods
void beginStrip(Adafruit_DotStar *strip){
  strip->begin(); // Initialize pins for output
  setAll(strip, 0,0,0); // Set all led to off
  strip->show();  // Turn all LEDs off ASAP
}

void setPixel(Adafruit_DotStar *strip, int Pixel, byte red, byte green, byte blue) {
   strip->setPixelColor(Pixel, strip->Color(red, green, blue));
 
}

void setAll(Adafruit_DotStar *strip, byte red, byte green, byte blue) {
  for(int i = 0; i < NUMPIXELS; i++ ) {
    setPixel(strip, i, red, green, blue); 
  }
    strip->show();
}
