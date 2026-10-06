#include "DMD3_ESP32.h"
#include "esp_timer.h"
#include <string.h>
#include <stdlib.h>

// Tabel pembalik bit (flip bits) untuk panel baris genap yang terbalik/daisy-chain
static const uint8_t flipBits[256] PROGMEM = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0,
    0x30, 0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
    0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4,
    0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC,
    0x3C, 0xBC, 0x7C, 0xFC, 0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
    0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA,
    0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6,
    0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
    0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81, 0x41, 0xC1,
    0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
    0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
    0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD,
    0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3,
    0x33, 0xB3, 0x73, 0xF3, 0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
    0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7,
    0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF,
    0x3F, 0xBF, 0x7F, 0xFF
};

DMD3::DMD3(int widthPanels, int heightPanels)
    : Bitmap(widthPanels * DMD_NUM_COLUMNS, heightPanels * DMD_NUM_ROWS)
    , _doubleBuffer(false)
    , phase(0)
    , fb0(0)
    , fb1(0)
    , displayfb(0)
    , _brightness(50)
    , _running(false)
    , _spi(NULL)
    , _timerHandle(NULL)
{
    fb0 = displayfb = fb;
}

DMD3::~DMD3()
{
    end();
    if (fb0) free(fb0);
    if (fb1) free(fb1);
    fb = 0;
    if (_spi) {
        delete _spi;
        _spi = NULL;
    }
}

void DMD3::timerCallback(void *arg) {
    DMD3 *instance = (DMD3 *)arg;
    if (instance && instance->_running) {
        instance->refresh();
    }
}

void DMD3::begin()
{
    pinMode(DMD_PIN_A, OUTPUT);
    pinMode(DMD_PIN_B, OUTPUT);
    pinMode(DMD_PIN_SCLK, OUTPUT);
    pinMode(DMD_PIN_OE, OUTPUT);

    digitalWrite(DMD_PIN_A, LOW);
    digitalWrite(DMD_PIN_B, LOW);
    digitalWrite(DMD_PIN_SCLK, LOW);
    digitalWrite(DMD_PIN_OE, LOW);

    // Inisialisasi SPI Master pada VSPI ESP32
    _spi = new SPIClass(VSPI);
    _spi->begin(DMD_PIN_CLK, -1, DMD_PIN_DATA, -1);
    _spi->setFrequency(10000000); // 10 MHz
    _spi->setDataMode(SPI_MODE0);
    _spi->setBitOrder(MSBFIRST);

    // Setup LEDC PWM untuk pin OE (kontrol kecerahan)
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(DMD_PIN_OE, 5000, 10); // 5 kHz, resolusi 10-bit (0-1023)
    #else
    ledcSetup(0, 5000, 10);
    ledcAttachPin(DMD_PIN_OE, 0);
    #endif

    setBrightness(_brightness);

    _running = true;

    // Timer scanning refresh presisi tinggi menggunakan esp_timer
    esp_timer_create_args_t timerArgs = {
        .callback = timerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "dmd_scan",
        .skip_unhandled_events = true
    };
    esp_timer_handle_t th;
    if (esp_timer_create(&timerArgs, &th) == ESP_OK) {
        _timerHandle = (void *)th;
        esp_timer_start_periodic(th, 2000); // Tiap 2000 us (2.0 ms)
    }
}

void DMD3::end()
{
    _running = false;
    if (_timerHandle) {
        esp_timer_stop((esp_timer_handle_t)_timerHandle);
        esp_timer_delete((esp_timer_handle_t)_timerHandle);
        _timerHandle = NULL;
    }
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(DMD_PIN_OE, 0);
    #else
    ledcWrite(0, 0);
    #endif
}

void DMD3::setBrightness(uint16_t bright)
{
    if (bright > 1023) bright = 1023;
    _brightness = bright;
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(DMD_PIN_OE, _brightness);
    #else
    ledcWrite(0, _brightness);
    #endif
}

void DMD3::setDoubleBuffer(bool doubleBuffer)
{
    if (doubleBuffer != _doubleBuffer) {
        _doubleBuffer = doubleBuffer;
        if (doubleBuffer) {
            unsigned int size = _stride * _height;
            fb1 = (uint8_t *)malloc(size);
            if (fb1) {
                memset(fb1, 0xFF, size);
                fb = fb1;
                displayfb = fb0;
            } else {
                _doubleBuffer = false;
            }
        } else if (fb1) {
            fb = fb0;
            displayfb = fb0;
            free(fb1);
            fb1 = 0;
        }
    }
}

void DMD3::swapBuffers()
{
    if (_doubleBuffer) {
        if (fb == fb0) {
            fb = fb1;
            displayfb = fb0;
        } else {
            fb = fb0;
            displayfb = fb1;
        }
    }
}

void DMD3::swapBuffersAndCopy()
{
    swapBuffers();
    if (_doubleBuffer) {
        memcpy(fb, displayfb, _stride * _height);
    }
}

void DMD3::refresh()
{
    if (!_spi || !_running) return;

    int stride4 = _stride * 4;
    uint8_t *data0, *data1, *data2, *data3;
    bool flipRow = ((_height & 0x10) == 0);

    // Buffer pengiriman data SPI
    uint8_t spiBuf[256];
    int bufIdx = 0;

    for (int y = 0; y < _height; y += 16) {
        if (!flipRow) {
            data0 = displayfb + _stride * (y + phase);
            data1 = data0 + stride4;
            data2 = data1 + stride4;
            data3 = data2 + stride4;
            for (int x = _stride; x > 0; --x) {
                spiBuf[bufIdx++] = *data3++;
                spiBuf[bufIdx++] = *data2++;
                spiBuf[bufIdx++] = *data1++;
                spiBuf[bufIdx++] = *data0++;
            }
            flipRow = true;
        } else {
            data0 = displayfb + _stride * (y + 16 - phase) - 1;
            data1 = data0 - stride4;
            data2 = data1 - stride4;
            data3 = data2 - stride4;
            for (int x = _stride; x > 0; --x) {
                spiBuf[bufIdx++] = pgm_read_byte(&(flipBits[*data3--]));
                spiBuf[bufIdx++] = pgm_read_byte(&(flipBits[*data2--]));
                spiBuf[bufIdx++] = pgm_read_byte(&(flipBits[*data1--]));
                spiBuf[bufIdx++] = pgm_read_byte(&(flipBits[*data0--]));
            }
            flipRow = false;
        }
    }

    // Kirim data pixel sekaligus via hardware SPI
    _spi->writeBytes(spiBuf, bufIdx);

    // Matikan OE sejenak saat perpindahan baris (menghilangkan bayangan / ghosting)
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(DMD_PIN_OE, 0);
    #else
    ledcWrite(0, 0);
    #endif

    // Pulsa latch
    digitalWrite(DMD_PIN_SCLK, HIGH);
    digitalWrite(DMD_PIN_SCLK, LOW);

    // Aktifkan baris A dan B
    digitalWrite(DMD_PIN_A, (phase & 0x01) ? HIGH : LOW);
    digitalWrite(DMD_PIN_B, (phase & 0x02) ? HIGH : LOW);

    // Pulihkan kecerahan OE PWM
    #if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(DMD_PIN_OE, _brightness);
    #else
    ledcWrite(0, _brightness);
    #endif

    phase = (phase + 1) & 0x03;
}

DMD3::Color DMD3::fromRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return (r || g || b) ? White : Black;
}
