#ifndef DMD3_ESP32_h
#define DMD3_ESP32_h

#include <Arduino.h>
#include <SPI.h>
#include "Bitmap.h"

// ==============================================================================
// Definisi Pin P10 HUB12 untuk ESP32 (Bebas Konflik dengan I2C DS3231)
// ==============================================================================
#ifndef DMD_PIN_OE
#define DMD_PIN_OE       15   // Output Enable (LEDC PWM Brightness)
#endif
#ifndef DMD_PIN_A
#define DMD_PIN_A        19   // Baris A
#endif
#ifndef DMD_PIN_B
#define DMD_PIN_B        27   // Baris B (GPIO 27 agar GPIO 21 bebas untuk I2C SDA)
#endif
#ifndef DMD_PIN_CLK
#define DMD_PIN_CLK      18   // VSPI SCK
#endif
#ifndef DMD_PIN_SCLK
#define DMD_PIN_SCLK     2    // Shift Register Latch (LAT / SCLK)
#endif
#ifndef DMD_PIN_DATA
#define DMD_PIN_DATA     23   // VSPI MOSI (R)
#endif

#define DMD_NUM_COLUMNS  32   // Lebar pixel per panel P10
#define DMD_NUM_ROWS     16   // Tinggi pixel per panel P10

class DMD3 : public Bitmap
{
public:
    explicit DMD3(int widthPanels = 1, int heightPanels = 1);
    ~DMD3();

    void begin();
    void end();

    bool doubleBuffer() const { return _doubleBuffer; }
    void setDoubleBuffer(bool doubleBuffer);
    void swapBuffers();
    void swapBuffersAndCopy();

    void refresh();
    void setBrightness(uint16_t bright); // 0 - 1023

    static Color fromRGB(uint8_t r, uint8_t g, uint8_t b);

private:
    DMD3(const DMD3 &other) : Bitmap(other) {}
    DMD3 &operator=(const DMD3 &) { return *this; }

    bool _doubleBuffer;
    uint8_t phase;
    uint8_t *fb0;
    uint8_t *fb1;
    uint8_t *displayfb;
    uint16_t _brightness;
    bool _running;

    SPIClass *_spi;
    void *_timerHandle;

    static void timerCallback(void *arg);
};

#endif
