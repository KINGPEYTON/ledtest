#include <Adafruit_DotStar.h>
#include <bluefruit.h>
#include <Adafruit_ISM330DHCX.h>

// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>

#define NUMPIXELS 72 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
#define DATAPIN 7  // green
#define CLOCKPIN 9 // yellow
Adafruit_DotStar strip1(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

// Here's how to control the LEDs from any two pins:
#define DATAPIN2 10  // green
#define CLOCKPIN2 12 // yellow
Adafruit_DotStar strip2(NUMPIXELS, DATAPIN2, CLOCKPIN2, DOTSTAR_BRG);

// BLE Service
BLEDfu bledfu;
BLEDis bledis;
BLEUart bleuart;

Adafruit_ISM330DHCX ism330dhcx;

void setup()
{

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  Serial.begin(115200);

  Serial.println("Adafruit Bluefruit Neopixel Test");
  Serial.println("--------------------------------");

  Serial.println();
  Serial.println("Please connect using the Bluefruit Connect LE application");

  // Init Bluefruit
  Bluefruit.begin();
  Bluefruit.setName("Dicks For Harambe");
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values

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
  beginStrip(&strip2);

  strip1.setBrightness(25);
  strip2.setBrightness(25);

  if (!ism330dhcx.begin_I2C()) {
    Serial.println("Failed to find ISM330DHCX chip");
  } else {
    Serial.println("ISM330DHCX Found!");
    beginIMUunit(&ism330dhcx, LSM6DS_ACCEL_RANGE_2_G, LSM6DS_GYRO_RANGE_250_DPS, LSM6DS_RATE_12_5_HZ, LSM6DS_RATE_12_5_HZ);
  }
}

int head = 0, tail = -10;  // Index of first 'on' and 'off' pixels
uint32_t color = 0xFF0000; // 'On' color (starts red)
uint32_t colors[] = {0x8a1a1a, 0x32a838, 0x0011d1, 0xfbff00, 0xff00fb, 0x00f7ff};

#define numModes 15 // how many modes there are

bool editMode = 0;

bool on1 = true;
bool on2 = true;
int mode1 = 7;
int mode2 = 2;

int prevCom;
bool buttonUp = true;

void loop()
{
  if (Bluefruit.connected() && bleuart.notifyEnabled())
  {
    int command = bleuart.read();
    if (command > 1)
    {
      if (editMode == 0)
      {
        mode1 = changeLedMode(mode1, command, &on1);
      }
      else if (editMode == 1)
      {
        mode2 = changeLedMode(mode2, command, &on2);
      }
      Serial.print((char)command);
    }
  }

  if (on1)
  {
    setLEDMode(mode1, &strip1);
  }
  if (on2)
  {
    setLEDMode(mode2, &strip2);
  }
}

//
void setLEDMode(int mode, Adafruit_DotStar *strip)
{
  switch (mode)
  {
  case 0:
  { // Get Version
    stripMode(strip);
    break;
  }
  case 1:
  { // Get Version
    raveMode(strip);
    break;
  }
  case 2:
  { // Get Version
    randomChaos(strip);
    break;
  }
  case 3:
  { // Get Version
    meteorRainGradient(strip, colors[random(0, 6)], 10, 32, true, random(5, 60), random(10, 70));
    break;
  }
  case 4:
  { // Get Version
    meteorRain(strip, 0x6c, 0x38, 0xc7, 10, 64, true, 30);
    break;
  }
  case 5:
  { // Get Version
    BouncingBalls(strip, 0xff, 0, 0, 3);
    break;
  }
  case 6:
  { // Get Version
    byte bColors[3][3] = {{0xff, 0, 0}, {0xff, 0xff, 0xff}, {0, 0, 0xff}};
    BouncingColoredBalls(strip, 3, bColors);
    break;
  }
  case 7:
  { // Get Version
    Fire(strip, 55, 120, 15);
    break;
  }
  case 8:
  { // Get Version
    theaterChaseRainbow(strip, 50);
    break;
  }
  case 9:
  { // Get Version
    gamerRGB(strip, 30);
    break;
  }
  case 10:
  { // Get Version
    Strobe(strip, 0x00, 0xff, 0x00, 10, 50, 50, 1500);
    break;
  }
  case 11:
  { // Get Version
    uint32_t tColors[] = {0x00ff00, 0x0000ff};
    multiColorStrobe(strip, tColors, sizeof(tColors) / sizeof(tColors[0]), 3, 10, 50, 250);
    break;
  }
  case 12:
  { // Get Version
    morseMessage(strip, "lol", 1000, 0xff00f0);
    break;
  }
  case 13:
  { // Get Version
    accelLed(strip);
    break;
  }
  case 14:
  { // Get Version
    gyroLed(strip);
    break;
  }
  case 15:
  { // Get Version
    tempLed(strip);
    break;
  }
  }
}

int changeLedMode(int mode, int command, bool *isOn)
{
  Serial.println((char)prevCom);
  if (prevCom == 'B')
  {
    if (isOn)
    {
      if (command == '7')
      {
        if (buttonUp)
        {
          mode--;
          if (mode < 0)
          {
            mode = numModes;
          }
          buttonUp = false;
        }
        else
        {
          buttonUp = true;
        }
      }
      else if (command == '8')
      {
        if (buttonUp)
        {
          mode++;
          if (mode > numModes)
          {
            mode = 0;
          }
          buttonUp = false;
        }
        else
        {
          buttonUp = true;
        }
      }
    }
    else if (command == '5')
    {
      if (buttonUp)
      {
        if (isOn)
        {
          setAll(&strip1, 0, 0, 0); // Set all led to off
          *isOn = false;
        }
        else
        {
          *isOn = true;
        }

        buttonUp = false;
      }
      else
      {
        buttonUp = true;
      }
    }
    else if (command == '6')
    {
      if (buttonUp)
      {
        editMode = !editMode;
        buttonUp = false;
      }
      else
      {
        buttonUp = true;
      }
    }
  }

  prevCom = command;
  return mode;
}

// modes
void stripMode(Adafruit_DotStar *strip)
{
  strip->setPixelColor(head, color); // 'On' pixel at head
  strip->setPixelColor(tail, 0);     // 'Off' pixel at tail
  strip->show();                     // Refresh strip
  delay(20);                         // Pause 20 milliseconds (~50 FPS)

  if (++head >= NUMPIXELS)
  {                         // Increment head index.  Off end of strip?
    head = 0;               //  Yes, reset head index to start
    if ((color >>= 8) == 0) //  Next color (R->G->B) ... past blue now?
      color = 0xFF0000;     //   Yes, reset to red
  }
  if (++tail >= NUMPIXELS)
    tail = 0; // Increment, reset tail index
}

void randomChaos(Adafruit_DotStar *strip)
{
  int randPix = random(NUMPIXELS);
  int randColor = random(16777216);

  strip->setPixelColor(randPix, strip->gamma32(strip->ColorHSV(randColor))); // 'Off' pixel at tail
  strip->show();

  delay(2); // Pause 20 milliseconds (~50 FPS)
}

void raveMode(Adafruit_DotStar *strip)
{
  // strip->setBrightness(64);
  uint32_t col = colors[random(0, 6)];
  for (int i = 0; i < NUMPIXELS; i++)
  {
    strip->setPixelColor(i, col); // 'On' pixel at head
  }
  strip->show(); // Refresh strip
  delay(300);
}

void meteorRainGradient(Adafruit_DotStar *strip, uint16_t startColor, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay, int colorSpeed)
{
  // setAll(strip, 0,0,0);
  uint16_t mc = startColor;

  for (int i = 0; i < NUMPIXELS + NUMPIXELS; i++)
  {

    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }

    // fade brightness all LEDs one step
    for (int j = 0; j < NUMPIXELS; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        fadeToBlack(strip, j, meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < NUMPIXELS) && (i - j >= 0))
      {
        strip->setPixelColor(i - j, strip->gamma32(strip->ColorHSV(mc, 255, 255)));
        mc += colorSpeed;
        // setPixel(i-j, red, green, blue);
      }
    }

    strip->show();
    delay(SpeedDelay);
  }
}

void meteorRain(Adafruit_DotStar *strip, byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay)
{
  // setAll(strip, 0,0,0);

  for (int i = 0; i < NUMPIXELS + NUMPIXELS; i++)
  {

    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }

    // fade brightness all LEDs one step
    for (int j = 0; j < NUMPIXELS; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        fadeToBlack(strip, j, meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < NUMPIXELS) && (i - j >= 0))
      {
        setPixel(strip, i - j, red, green, blue);
        // setPixel(i-j, red, green, blue);
      }
    }

    strip->show();
    delay(SpeedDelay);
  }
}

void BouncingBalls(Adafruit_DotStar *strip, byte red, byte green, byte blue, int BallCount)
{
  float Gravity = -9.81;
  int StartHeight = 1;

  float Height[BallCount];
  float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int Position[BallCount];
  long ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];

  for (int i = 0; i < BallCount; i++)
  {
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
  }

  while (true)
  {
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
    for (int i = 0; i < BallCount; i++)
    {
      TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

      if (Height[i] < 0)
      {
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();

        if (ImpactVelocity[i] < 0.01)
        {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round(Height[i] * (NUMPIXELS - 1) / StartHeight);
    }

    for (int i = 0; i < BallCount; i++)
    {
      setPixel(strip, Position[i], red, green, blue);
    }

    strip->show();
    setAll(strip, 0, 0, 0);
  }
}

void BouncingColoredBalls(Adafruit_DotStar *strip, int BallCount, byte colors[][3])
{
  float Gravity = -9.81;
  int StartHeight = 1;

  float Height[BallCount];
  float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int Position[BallCount];
  long ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];

  for (int i = 0; i < BallCount; i++)
  {
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i) / pow(BallCount, 2);
  }

  while (true)
  {
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
    for (int i = 0; i < BallCount; i++)
    {
      TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

      if (Height[i] < 0)
      {
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();

        if (ImpactVelocity[i] < 0.01)
        {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round(Height[i] * (NUMPIXELS - 1) / StartHeight);
    }

    for (int i = 0; i < BallCount; i++)
    {
      setPixel(strip, Position[i], colors[i][0], colors[i][1], colors[i][2]);
    }

    strip->show();
    setAll(strip, 0, 0, 0);
  }
}

void Fire(Adafruit_DotStar *strip, int Cooling, int Sparking, int SpeedDelay)
{
  static byte heat[NUMPIXELS];
  int cooldown;

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < NUMPIXELS; i++)
  {
    cooldown = random(0, ((Cooling * 10) / NUMPIXELS) + 2);

    if (cooldown > heat[i])
    {
      heat[i] = 0;
    }
    else
    {
      heat[i] = heat[i] - cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = NUMPIXELS - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if (random(255) < Sparking)
  {
    int y = random(7);
    heat[y] = heat[y] + random(160, 255);
    // heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for (int j = 0; j < NUMPIXELS; j++)
  {
    setPixelHeatColor(strip, NUMPIXELS - j, heat[j]);
  }

  strip->show();
  delay(SpeedDelay);
}

void theaterChaseRainbow(Adafruit_DotStar *strip, int SpeedDelay)
{
  byte *c;

  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
    for (int q = 0; q < 3; q++)
    {
      for (int i = 0; i < NUMPIXELS; i += 3)
      {
        c = Wheel((i + j) % 255);
        setPixel(strip, i + q, *c, *(c + 1), *(c + 2)); // turn every third pixel on
      }
      strip->show();

      delay(SpeedDelay);

      for (int i = 0; i < NUMPIXELS; i = i + 3)
      {
        setPixel(strip, i + q, 0, 0, 0); // turn every third pixel off
      }
    }
  }
}

void gamerRGB(Adafruit_DotStar *strip, int SpeedDelay)
{
  byte *c;

  for (int j = 0; j < 256; j++)
  { // cycle all 256 colors in the wheel
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
    for (int i = 0; i < NUMPIXELS; i++)
    {
      c = Wheel((i + j) % 255);
      setPixel(strip, i, *c, *(c + 1), *(c + 2)); // turn every third pixel on
    }
    strip->show();

    delay(SpeedDelay);
  }
}

void Strobe(Adafruit_DotStar *strip, byte red, byte green, byte blue, int StrobeCount, int FlashDelay, int inbeweenDelay, int EndPause)
{
  for (int j = 0; j < StrobeCount; j++)
  {
    setAll(strip, red, green, blue);
    strip->show();
    delay(FlashDelay);
    setAll(strip, 0, 0, 0);
    strip->show();
    delay(inbeweenDelay);

    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
  }

  delay(EndPause);
}

void Strobe(Adafruit_DotStar *strip, uint32_t col, int StrobeCount, int FlashDelay, int inbeweenDelay, int EndPause)
{
  for (int j = 0; j < StrobeCount; j++)
  {
    setAll(strip, col);
    strip->show();
    delay(FlashDelay);
    setAll(strip, 0, 0, 0);
    strip->show();
    delay(inbeweenDelay);

    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
  }

  delay(EndPause);
}

void multiColorStrobe(Adafruit_DotStar *strip, uint32_t col[], int arrSize, int sects, int StrobeCount, int FlashDelay, int EndPause)
{
  int sections = arrSize * sects;
  for (int i = 0; i < arrSize; i++)
  {
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }
    for (int j = 0; j < StrobeCount; j++)
    {
      for (int h = i; h < sections; h += arrSize)
      {
        setSect(strip, col[i], (NUMPIXELS / sections) * h, (NUMPIXELS / sections) * (h + 1));
      }
      strip->show();
      delay(FlashDelay);
      setAll(strip, 0, 0, 0);
      strip->show();
      delay(FlashDelay);
      if (bleuart.read() == 'B')
      {
        prevCom = 'B';
        break;
      }
    }
    delay(EndPause);
  }
}

void morseMessage(Adafruit_DotStar *strip, String message, int timeDelay, uint32_t col)
{
  int num = message.length();
  char *char_array = new char[num + 1];
  strcpy(char_array, message.c_str());

  for (int i = 0; i < num; i++)
  {
    if (bleuart.read() == 'B')
    {
      prevCom = 'B';
      break;
    }

    morseLetter(strip, char_array[i], col, timeDelay);
  }

  delay(timeDelay * 3);
}

void accelLed(Adafruit_DotStar *strip){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  ism330dhcx.getEvent(&accel, &gyro, &temp);

  int colB = (accel.acceleration.x * 10);
  int colR = (accel.acceleration.y * 10);
  int colG = (accel.acceleration.z * 10);

  setAll(strip, (byte)colB, (byte)colR, (byte)colB);
}

void gyroLed(Adafruit_DotStar *strip){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  ism330dhcx.getEvent(&accel, &gyro, &temp);

  int colB = (gyro.gyro.x * 10);
  int colR = (gyro.gyro.y * 10);
  int colG = (gyro.gyro.z * 10);

  setAll(strip, (byte)colB, (byte)colR, (byte)colB);
}

void tempLed(Adafruit_DotStar *strip){
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  ism330dhcx.getEvent(&accel, &gyro, &temp);

  int colB = 255;
  int colR = 255;
  int colG = 255;

    if (temp.temperature < -10)
    {
      colB = 255;
      colR = 255;
      colG = 255;
    } else if (temp.temperature < 5)
    {
      colB = 255;
      colR = 255 - ((temp.temperature + 10) * 15);
      colG = 255 - ((temp.temperature + 10) * 15);
    }
    else if (temp.temperature < 20)
    {
      colB = 255 - ((temp.temperature - 5) * 15);
      colR = 0;
      colG = ((temp.temperature - 5) * 15);
    }
    else if (temp.temperature < 35)
    {
      colB = 0;
      colR = ((temp.temperature - 20) * 15);
      colG = 255 - ((temp.temperature - 20) * 15);
    }
    else if (temp.temperature < 50)
    {
      colB = ((temp.temperature - 35) * 15);
      colR = 255;
      colG = ((temp.temperature - 35) * 15);
    } else {
      colB = 0;
      colR = 0;
      colG = 0;
    }
  
  Serial.print("\n\tTemperature: ");
  Serial.print(temp.temperature);
  Serial.println(" deg C");

  Serial.println("Colors are:");
  Serial.printf("R: %d \t G: %d \t B: %d", colR, colG, colB);

  setAll(strip, (byte)colB, (byte)colR, (byte)colB);

  //delay(200);
}
// mode helper methods
void fadeToBlack(Adafruit_DotStar *strip, int ledNo, byte fadeValue)
{
  uint32_t oldColor;
  uint8_t r, g, b;
  int value;

  oldColor = strip->getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r = (r <= 10) ? 0 : (int)r - (r * fadeValue / 256);
  g = (g <= 10) ? 0 : (int)g - (g * fadeValue / 256);
  b = (b <= 10) ? 0 : (int)b - (b * fadeValue / 256);

  strip->setPixelColor(ledNo, r, g, b);
}

void setPixelHeatColor(Adafruit_DotStar *strip, int Pixel, byte temperature)
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2;              // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if (t192 > 0x80)
  { // hottest
    setPixel(strip, Pixel, heatramp, 255, 255);
  }
  else if (t192 > 0x40)
  { // middle
    setPixel(strip, Pixel, 0, 255, heatramp);
  }
  else
  { // coolest
    setPixel(strip, Pixel, 0, heatramp, 0);
  }
}

byte *Wheel(byte WheelPos)
{
  static byte c[3];

  if (WheelPos < 85)
  {
    c[0] = WheelPos * 3;
    c[1] = 255 - WheelPos * 3;
    c[2] = 0;
  }
  else if (WheelPos < 170)
  {
    WheelPos -= 85;
    c[0] = 255 - WheelPos * 3;
    c[1] = 0;
    c[2] = WheelPos * 3;
  }
  else
  {
    WheelPos -= 170;
    c[0] = 0;
    c[1] = WheelPos * 3;
    c[2] = 255 - WheelPos * 3;
  }

  return c;
}

void morseLetter(Adafruit_DotStar *strip, char letter, uint32_t col, int letterDelay)
{
  switch (letter)
  {
  case 'a':
  {                                                             //  .-
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'b':
  {                                                             ///  -...
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 3, letterDelay / 4, letterDelay / 2, 0); // 3 dot
    break;
  }
  case 'c':
  {                                                             //  -.-.
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    break;
  }
  case 'd':
  {                                                             //  -..
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dot
    break;
  }
  case 'e':
  {                                                             //  .
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    break;
  }
  case 'f':
  {                                                             //  ..-.
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dot
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    break;
  }
  case 'g':
  {                                                             //  --.
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    break;
  }
  case 'h':
  {                                                             //  ....
    Strobe(strip, col, 4, letterDelay / 4, letterDelay / 2, 0); // 4 dot
    break;
  }
  case 'i':
  {                                                             //  ..
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dot
    break;
  }
  case 'j':
  {                                                             //  .---
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dot
    break;
  }
  case 'k':
  {                                                             //  -.-
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'l':
  {                                                             //  .-..
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dot
    break;
  }
  case 'm':
  {                                                             //  --
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    break;
  }
  case 'n':
  {                                                             //  -.
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dot
    break;
  }
  case 'o':
  {                                                             //  ---
    Strobe(strip, col, 3, letterDelay / 2, letterDelay / 2, 0); // 3 dash
    break;
  }
  case 'p':
  {                                                             //  .--.
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    break;
  }
  case 'q':
  {                                                             //  --.-
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'r':
  {                                                             //  .-.
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    break;
  }
  case 's':
  {                                                             //  ...
    Strobe(strip, col, 3, letterDelay / 4, letterDelay / 2, 0); // 3 dots
    break;
  }
  case 't':
  {                                                             //  -
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'u':
  {                                                             //  ..-
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'v':
  {                                                             //  ...-
    Strobe(strip, col, 3, letterDelay / 4, letterDelay / 2, 0); // 3 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'w':
  {                                                             //  .--
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    break;
  }
  case 'x':
  {                                                             //  -..-
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case 'y':
  {                                                             //  -.--
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    break;
  }
  case 'z':
  {                                                             //  --..
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dots
    break;
  }
  case '1':
  {                                                             //  .----
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dots
    Strobe(strip, col, 4, letterDelay / 2, letterDelay / 2, 0); // 4 dash
    break;
  }
  case '2':
  {                                                             //  ..---
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dots
    Strobe(strip, col, 3, letterDelay / 2, letterDelay / 2, 0); // 3 dash
    break;
  }
  case '3':
  {                                                             //  ...--
    Strobe(strip, col, 3, letterDelay / 4, letterDelay / 2, 0); // 3 dots
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dash
    break;
  }
  case '4':
  {                                                             //  ....-
    Strobe(strip, col, 4, letterDelay / 4, letterDelay / 2, 0); // 4 dots
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dash
    break;
  }
  case '5':
  {                                                             //  .....
    Strobe(strip, col, 5, letterDelay / 4, letterDelay / 2, 0); // 5 dots
    break;
  }
  case '6':
  {                                                             //  -....
    Strobe(strip, col, 1, letterDelay / 4, letterDelay / 2, 0); // 1 dash
    Strobe(strip, col, 4, letterDelay / 2, letterDelay / 2, 0); // 4 dots
    break;
  }
  case '7':
  {                                                             //  --...
    Strobe(strip, col, 2, letterDelay / 4, letterDelay / 2, 0); // 2 dash
    Strobe(strip, col, 3, letterDelay / 2, letterDelay / 2, 0); // 3 dots
    break;
  }
  case '8':
  {                                                             //  ---..
    Strobe(strip, col, 3, letterDelay / 4, letterDelay / 2, 0); // 3 dash
    Strobe(strip, col, 2, letterDelay / 2, letterDelay / 2, 0); // 2 dots
    break;
  }
  case '9':
  {                                                             //  ----.
    Strobe(strip, col, 4, letterDelay / 4, letterDelay / 2, 0); // 4 dash
    Strobe(strip, col, 1, letterDelay / 2, letterDelay / 2, 0); // 1 dots
    break;
  }
  case '0':
  {                                                             //  -----
    Strobe(strip, col, 5, letterDelay / 4, letterDelay / 2, 0); // 5 dash
    break;
  }
  case ' ':
  { //
    delay(letterDelay * 2);
    break;
  }
  }
  delay(letterDelay);
}

// bluetooth methods
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection *connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = {0};
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
  Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
  Bluefruit.Advertising.start(0);             // 0 = Don't stop advertising after n seconds
}

void beginIMUunit(Adafruit_ISM330DHCX *IMUunit, accel_range accR, gyro_range gyroR, data_rate accRate, data_rate grypRate){
  IMUunit->setAccelRange(accR);
  Serial.print("Accelerometer range set to: ");
  switch (IMUunit->getAccelRange()) {
  case LSM6DS_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case LSM6DS_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case LSM6DS_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case LSM6DS_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }

  IMUunit->setGyroRange(gyroR);
  Serial.print("Gyro range set to: ");
  switch (IMUunit->getGyroRange()) {
  case LSM6DS_GYRO_RANGE_125_DPS:
    Serial.println("125 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case LSM6DS_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  case ISM330DHCX_GYRO_RANGE_4000_DPS:
    Serial.println("4000 degrees/s");
    break;
  }

  IMUunit->setAccelDataRate(accRate);
  Serial.print("Accelerometer data rate set to: ");
  switch (IMUunit->getAccelDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  IMUunit->setGyroDataRate(grypRate);
  Serial.print("Gyro data rate set to: ");
  switch (IMUunit->getGyroDataRate()) {
  case LSM6DS_RATE_SHUTDOWN:
    Serial.println("0 Hz");
    break;
  case LSM6DS_RATE_12_5_HZ:
    Serial.println("12.5 Hz");
    break;
  case LSM6DS_RATE_26_HZ:
    Serial.println("26 Hz");
    break;
  case LSM6DS_RATE_52_HZ:
    Serial.println("52 Hz");
    break;
  case LSM6DS_RATE_104_HZ:
    Serial.println("104 Hz");
    break;
  case LSM6DS_RATE_208_HZ:
    Serial.println("208 Hz");
    break;
  case LSM6DS_RATE_416_HZ:
    Serial.println("416 Hz");
    break;
  case LSM6DS_RATE_833_HZ:
    Serial.println("833 Hz");
    break;
  case LSM6DS_RATE_1_66K_HZ:
    Serial.println("1.66 KHz");
    break;
  case LSM6DS_RATE_3_33K_HZ:
    Serial.println("3.33 KHz");
    break;
  case LSM6DS_RATE_6_66K_HZ:
    Serial.println("6.66 KHz");
    break;
  }

  IMUunit->configInt1(false, false, true); // accelerometer DRDY on INT1
  IMUunit->configInt2(false, true, false); // gyro DRDY on INT2
}

// strip help methods
void beginStrip(Adafruit_DotStar *strip)
{
  strip->begin();         // Initialize pins for output
  setAll(strip, 0, 0, 0); // Set all led to off
  strip->show();          // Turn all LEDs off ASAP
}

void setPixel(Adafruit_DotStar *strip, int Pixel, byte red, byte green, byte blue)
{
  strip->setPixelColor(Pixel, strip->gamma32(strip->Color(red, green, blue)));
}

void setAll(Adafruit_DotStar *strip, byte red, byte green, byte blue)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    setPixel(strip, i, red, green, blue);
  }
  strip->show();
}

void setAll(Adafruit_DotStar *strip, uint32_t colo)
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    strip->setPixelColor(i, colo);
  }
  strip->show();
}

void setSect(Adafruit_DotStar *strip, uint32_t colo, int start, int end)
{
  if (end > NUMPIXELS)
  {
    end = NUMPIXELS;
  }

  for (int i = start; i < end; i++)
  {
    strip->setPixelColor(i, colo);
  }
  strip->show();
}
