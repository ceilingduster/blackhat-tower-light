#include <M5Core2.h>
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void error_lights();

// Pattern types supported:
enum pattern
{
    NONE,
    ROTATING,
    FLASHING,
    FILL,
    FADEIN,
    RADAR,
    DOUBLE,
    SCANNING
};

#define PIN 25
#define NUMPIXELS 12
#define NUMRINGS 3
#define TOTALPIXELS NUMPIXELS *NUMRINGS
#define RGBTYPE NEO_GRBW
#define DELAYVAL 50

class PatternLib : public Adafruit_NeoPixel
{
public:
    unsigned long Interval[NUMRINGS];   // milliseconds between updates
    unsigned long lastUpdate[NUMRINGS]; // last update of position
    int16_t Index[NUMRINGS];            // current step within the pattern
    uint16_t TotalIndex;
    uint16_t numberPixels[NUMRINGS]; // number of pixels array
    uint32_t pixelColor[NUMRINGS];   // pixel color array
    pattern ActivePattern[NUMRINGS]; // which pattern is running

    void (*OnComplete)(); // Callback on completion of pattern
    void setBrightnessLevel(uint8_t BrightnessLevel);
    void Update();
    void Init();
    pattern GetPattern(long pattern_num);
    void Increment(int ringNumber);
    void TotalIncrement();
    void clearAllPixels();
    void clearPixels(int ringNumber);
    uint8_t Red(uint32_t color);
    uint8_t Green(uint32_t color);
    uint8_t Blue(uint32_t color);
    void updateLED(int ringNumber);
    uint32_t FadeColor(uint32_t color, uint16_t steps, uint16_t index);
    uint32_t DimColor(uint32_t color);

    PatternLib(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel, void (*aCallBack)());
};
