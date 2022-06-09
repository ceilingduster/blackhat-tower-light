#include <M5Core2.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "patternlib.h"
#include "screenlib.h"

PatternLib::PatternLib(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel, void (*aCallBack)()) : Adafruit_NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel)
{
  OnComplete = aCallBack;
  Init();
}

void PatternLib::Init()
{
  // initialize Index
  for (int r = 0; r <= NUMRINGS - 1; r++)
  {
    Index[r] = 0;
    numberPixels[r] = NUMPIXELS;
    Interval[r] = 500;
    ActivePattern[r] = NONE;
  }

  pixelColor[0] = Color(0, 0, 0);
  pixelColor[1] = Color(0, 0, 0);
  pixelColor[2] = Color(0, 0, 0);

  setBrightnessLevel(200);
}

pattern PatternLib::GetPattern(long pattern_num)
{
  switch (pattern_num)
  {
  case 7:
    return SCANNING;
    break;
  case 6:
    return DOUBLE;
    break;
  case 5:
    return RADAR;
    break;
  case 4:
    return FADEIN;
    break;
  case 3:
    return FILL;
    break;
  case 2:
    return FLASHING;
    break;
  case 1:
    return ROTATING;
    break;
  case 0:
    return NONE;
    break;
  default:
    return NONE;
    break;
  }
}

void PatternLib::setBrightnessLevel(uint8_t BrightnessLevel)
{
  setBrightness(BrightnessLevel);
}

void PatternLib::Update()
{
  for (int r = 0; r <= NUMRINGS - 1; r++)
  {
    if (Interval[r] > 0)
    {
      // is it time to update?
      if ((millis() - lastUpdate[r]) > Interval[r])
      {
        lastUpdate[r] = millis();
        updateLED(r);
        Increment(r);
        TotalIncrement();
      }
    }
  }
}

void PatternLib::Increment(int ringNumber)
{
  Index[ringNumber]++;
  if (Index[ringNumber] >= numberPixels[ringNumber])
  {
    Index[ringNumber] = 0;
  }
}

void PatternLib::TotalIncrement()
{
  TotalIndex++;
  if (TotalIndex >= TOTALPIXELS)
  {
    TotalIndex = 0;
  }
}

// clear all the pixels for all rings
void PatternLib::clearAllPixels()
{
  clear();
}

// clear all the pixels on a specific ring
void PatternLib::clearPixels(int ringNumber)
{
  uint16_t startPixel = ((numberPixels[ringNumber] * (ringNumber + 1)) - numberPixels[ringNumber]);
  uint16_t endPixel = startPixel + numberPixels[ringNumber] - 1;

  Serial.print("Clearing pixels between ");
  Serial.print(startPixel);
  Serial.print(" and ");
  Serial.println(endPixel);

  for (int i = startPixel; i <= endPixel; i++)
  {
    setPixelColor(i, Color(0, 0, 0));
  }
}

// Returns the Red component of a 32-bit color
uint8_t PatternLib::Red(uint32_t color)
{
  return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t PatternLib::Green(uint32_t color)
{
  return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t PatternLib::Blue(uint32_t color)
{
  return color & 0xFF;
}

// update the pixels on a ring for the current active pattern
void PatternLib::updateLED(int ringNumber)
{
  uint16_t offsetShift = ((numberPixels[ringNumber] * (ringNumber + 1)) - numberPixels[ringNumber]);
  uint16_t pixelNum = Index[ringNumber] + offsetShift;
  int16_t firsthalf;
  int16_t secondhalf;
  int16_t thirdhalf;
  int16_t fourthhalf;

  // clear all pixels
  switch (ActivePattern[ringNumber])
  {
  case NONE:
    clearPixels(ringNumber);
    break;
  case RADAR:
    clearPixels(ringNumber);

    firsthalf = pixelNum % (numberPixels[ringNumber] / 2) + offsetShift;
    secondhalf = (numberPixels[ringNumber] / 2) - firsthalf;
    thirdhalf = numberPixels[ringNumber] - firsthalf;
    fourthhalf = (numberPixels[ringNumber] / 2) + firsthalf;

    setPixelColor(firsthalf, pixelColor[ringNumber]);
    setPixelColor(secondhalf, pixelColor[ringNumber]);
    setPixelColor(thirdhalf, pixelColor[ringNumber]);
    setPixelColor(fourthhalf, pixelColor[ringNumber]);
    break;
  case ROTATING:
    clearPixels(ringNumber);
    setPixelColor(pixelNum, pixelColor[ringNumber]);
    break;
  case FLASHING:
    if ((Index[ringNumber] % 2) == 1)
    {
      clearPixels(ringNumber);
    }
    else
    {
      for (int i = offsetShift; i <= (offsetShift + numberPixels[ringNumber]) - 1; i++)
      {
        setPixelColor(i, pixelColor[ringNumber]);
      }
    }
    break;
  case FADEIN:
    clearPixels(ringNumber);
    for (int i = offsetShift; i <= (offsetShift + NUMPIXELS) - 1; i++)
    {
      setPixelColor(i, FadeColor(pixelColor[ringNumber], numberPixels[ringNumber], Index[ringNumber]));
    }
    break;
  case FILL:
    for (int i = offsetShift; i <= (offsetShift + numberPixels[ringNumber]) - 1; i++)
    {
      setPixelColor(i, pixelColor[ringNumber]);
    }
    break;
  case SCANNING:
    clearPixels(ringNumber);

    firsthalf = Index[ringNumber];
    secondhalf = firsthalf - firsthalf + numberPixels[ringNumber] - firsthalf;

    setPixelColor(firsthalf + offsetShift, pixelColor[ringNumber]);
    setPixelColor(secondhalf + offsetShift, pixelColor[ringNumber]);
    break;
  case DOUBLE:
    clearPixels(ringNumber);

    firsthalf = pixelNum % (numberPixels[ringNumber] / 2);
    secondhalf = firsthalf + (numberPixels[ringNumber] / 2);

    setPixelColor(firsthalf, pixelColor[ringNumber]);
    setPixelColor(secondhalf, pixelColor[ringNumber]);
    break;
  default:
    break;
  }

  show();
}

uint32_t PatternLib::FadeColor(uint32_t color, uint16_t steps, uint16_t index)
{
  uint8_t red = Red(color) * index / steps;
  uint8_t green = Green(color) * index / steps;
  uint8_t blue = Blue(color) * index / steps;
  return Color(red, green, blue);
}

uint32_t PatternLib::DimColor(uint32_t color)
{
  // Shift R, G and B components one bit to the right
  uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
  return dimColor;
}
