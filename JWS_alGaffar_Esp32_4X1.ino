/*************************************************************************************
 * JWS alGaffar 4x1 P10 LED Matrix (ESP32 Edition)
 * Firmware Jadwal Waktu Sholat Otomatis berbasis ESP32
 * Menggunakan library DMD3_ESP32 (Double-buffered, flicker-free VSPI driver),
 * RTC DS3231 via I2C (SDA: 21, SCL: 22), DFPlayer Mini via Hardware Serial2 (TX: 17, RX: 16),
 * Built-in Bluetooth Classic SPP untuk kontrol via aplikasi Android alGaffar.
 *************************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <DS3231.h>
#include <EEPROM.h>
#include <BluetoothSerial.h>
#include "DMD3_ESP32.h"
#include "font/BigNumber.h"
#include "font/Font4x6.h"
#include "font/System5x7.h"
#include "font/Font6x7.h"

// =========================================
// Definisi Pin Hardware ESP32 =============
// =========================================
#define BUZZ 4          // Buzzer aktif/pasif pada GPIO 4
#define I2C_SDA 21      // DS3231 RTC SDA
#define I2C_SCL 22      // DS3231 RTC SCL
#define MP3_RX 16       // DFPlayer TX -> ESP32 RX2 (GPIO 16)
#define MP3_TX 17       // ESP32 TX2 (GPIO 17) -> DFPlayer RX (via 1k resistor)

DMD3 Disp(4, 1);
RTClib RTC;
DS3231 Clock;
BluetoothSerial SerialBT;
HardwareSerial SerialMP3(2);

// Structure of Variable
typedef struct  // loaded to EEPROM
{
  uint8_t state;  // 1 byte  add 0
  float L_LA;     // 4 byte  add 1
  float L_LO;     // 4 byte  add 5
  float L_AL;     // 4 byte  add 9
  float L_TZ;     // 4 byte  add 13
  uint8_t MT;     // 1 byte  add 17  // value 1-masjid  2-mushollah 3-surau 4-langgar
  uint8_t BL;     // 1 byte  add 18
  uint8_t RT;     // 1 byte  add 19
  uint8_t IH;     // 1 byte  add 20
  uint8_t AD;     // 1 byte  add 21
  uint8_t SO;     // 1 byte  add 22
  uint8_t JM;     // 1 byte  add 23
  uint8_t I1;     // 1 byte  add 24
  uint8_t I4;     // 1 byte  add 25
  uint8_t I5;     // 1 byte  add 26
  uint8_t I6;     // 1 byte  add 27
  uint8_t I7;     // 1 byte  add 28
  uint8_t BZ;     // 1 byte  add 29
  uint8_t SI;     // 1 byte  add 30
  uint8_t ST;     // 1 byte  add 31
  uint8_t SU;     // 1 byte  add 32
  uint8_t IS;     // 1 byte  add 33
  uint8_t IL;     // 1 byte  add 34
  uint8_t IA;     // 1 byte  add 35
  uint8_t IM;     // 1 byte  add 36
  uint8_t II;     // 1 byte  add 37
  int8_t  CH;     // 1 byte  add 38
  uint8_t IN;     // 1 byte  add 39
} struct_param;

typedef struct
{
  uint8_t hD;
  uint8_t hM;
  uint16_t hY;
} hijir_date;

// Variable by Structure
struct_param Prm;
hijir_date nowH;

#define ADDR_MP3_PRM 880
#define MP3_PARAM_VERSION 101

typedef struct {
  uint8_t version;        // 1 byte  add 880
  uint8_t enable;         // 1 byte  add 881
  uint8_t volume;         // 1 byte  add 882
  uint8_t tartilMin[6];   // 6 byte  add 883-888 (Subuh, Dzuhur, Ashar, Maghrib, Isya, Jumat)
  uint8_t tartilTrack[6]; // 6 byte  add 889-894
  uint8_t tarhimTrack[6]; // 6 byte  add 895-900
  uint16_t tarhimSec[6];  // 12 byte add 901-912
} struct_mp3_prm;

struct_mp3_prm Mp3Prm;

void dfStop();
void dfSetVolume(uint8_t vol);
void dfPlayManual(uint8_t folder, uint8_t track);
void Disp_init();
void setBrightness(int bright);
void updateTime();
void update_All_data();
void check_azzan();
void check_mp3();
void serviceBluetooth();
void GetPrm();
void mp3_init();
void sholatCal();
hijir_date toHijri(uint16_t Y, uint8_t M, uint8_t D, uint8_t cor);
void setRunSel(int val);
void setJumat(bool val);
void fType(int x);
void Buzzer(uint8_t state);
void dwMrq(const char* msg, int Speed, int dDT, int DrawAdd);
void drawSholat(int DrawAdd);
void drawOnAzzan(int DrawAdd);
void drawAzzan(int DrawAdd);
void drawIqomah(int DrawAdd);
void blinkBlock(int DrawAdd);
char* drawWelcome();
char* drawDateH();
char* drawDateM();
char* drawInfo(uint16_t addr);

#define ADDR_JUMAT 1022
#define ADDR_RUNSEL 1023

// Time Variable
DateTime now;
float floatnow = 0;
uint8_t daynow = 0;
int8_t SholatNow = -1;
boolean jumat = false;
boolean azzan = false;
uint8_t reset_x = 0;
boolean rtcTimeValid = false;
boolean displayReady = false;
uint32_t lastRtcReadMs = 0;

// Other Variable
float sholatT[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t Iqomah[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

// Bluetooth & Display
char CH_Prm[155];
int DWidth = Disp.width();
int DHeight = Disp.height();
boolean DoSwap;
int RunSel = 1;
int RunFinish = 0;

// Helper update byte EEPROM untuk ESP32
void eeprom_update_byte(int address, uint8_t val) {
  if (EEPROM.read(address) != val) {
    EEPROM.write(address, val);
    EEPROM.commit();
  }
}

void setRunSel(int val) {
  if (RunSel == val) return;
  RunSel = val;
  if (val == 1 || (val >= 100 && val <= 104)) {
    eeprom_update_byte(ADDR_RUNSEL, (uint8_t)val);
  }
}

void setJumat(bool val) {
  if (jumat == val) return;
  jumat = val;
  eeprom_update_byte(ADDR_JUMAT, val ? 1 : 0);
}

void Disp_init() {
  Disp.setDoubleBuffer(true);
  displayReady = true;
  setBrightness((int)Prm.BL);
  fType(1);
  Disp.clear();
  Disp.swapBuffers();
}

void setBrightness(int bright) {
  // Prm.BL bernilai 0 - 255 dari aplikasi alGaffar.
  // Driver LEDC 10-bit menggunakan rentang 0 - 1023.
  uint16_t duty = (uint16_t)bright * 4;
  if (duty > 1023) duty = 1023;
  Disp.setBrightness(duty);
}

void updateTime() {
  const uint32_t currentMs = millis();
  if (rtcTimeValid && (uint32_t)(currentMs - lastRtcReadMs) < 1000UL) {
    return;
  }

  now = RTC.now();
  floatnow =
    (float)now.hour() + (float)now.minute() / 60.0f + (float)now.second() / 3600.0f;
  daynow = ((now.dayOfTheWeek() + 6) % 7) + 1;
  lastRtcReadMs = currentMs;
  rtcTimeValid = true;
}

void Timer_Minute(int repeat_time)
{
  static uint32_t lsRn;
  uint32_t Tmr = millis();
  if ((Tmr - lsRn) > ((uint32_t)repeat_time * 60000UL)) {
    lsRn = Tmr;
    update_All_data();
  }
}

void update_All_data() {
  uint8_t date_cor = 0;
  updateTime();
  sholatCal();
  if (floatnow > sholatT[6]) {
    date_cor = 1;
  }
  nowH = toHijri(now.year(), now.month(), now.day(), date_cor);

  if (displayReady) {
    if ((floatnow > 21.0f) || (floatnow < 3.5f)) {
      setBrightness(4);
    } else {
      setBrightness(Prm.BL);
    }
  }
}

void check_azzan() {
  static uint8_t lastAzzanDay = 0;
  static int8_t  lastAzzanPrayer = -1;
  SholatNow = -1;

  uint16_t currentMinute = (uint16_t)now.hour() * 60U + (uint16_t)now.minute();

  for (uint8_t i = 0; i < 8; i++) {
    if (i == 0 || i == 2 || i == 3) continue;

    uint16_t prayerMinute = (uint16_t)ceil((sholatT[i] * 60.0f) - 0.0001f);

    if (currentMinute >= prayerMinute) {
      SholatNow = i;
    }

    if (!azzan && (daynow != lastAzzanDay || i != lastAzzanPrayer) &&
        currentMinute >= prayerMinute && currentMinute < prayerMinute + 5U) {
      lastAzzanDay = daynow;
      lastAzzanPrayer = i;
      setJumat(daynow == 5 && i == 4 && Prm.MT == 1);
      SholatNow = i;
      azzan = true;
      dfStop();

      setRunSel(99);
      break;
    }
  }
}

// =======================================
// === SETUP =============================
// =======================================

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n[BOOT] JWS alGaffar ESP32 Edition Starting...");

  // Inisialisasi Bluetooth Classic SPP untuk koneksi aplikasi Android alGaffar
  SerialBT.begin("JWS-alGaffar");
  Serial.println("[BT] Bluetooth SPP Siap dengan nama: JWS-alGaffar");

  // Inisialisasi Hardware Serial2 untuk DFPlayer Mini
  SerialMP3.begin(9600, SERIAL_8N1, MP3_RX, MP3_TX);
  Serial.println("[MP3] Hardware Serial2 (DFPlayer) Siap");

  // Inisialisasi I2C (RTC DS3231)
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("[RTC] I2C RTC DS3231 Siap");

  // Inisialisasi EEPROM Flash (1024 bytes)
  EEPROM.begin(1024);
  Serial.println("[EEPROM] Flash EEPROM (1024 bytes) Siap");

  // Inisialisasi Buzzer
  pinMode(BUZZ, OUTPUT);
  tone(BUZZ, 1200, 150);
  delay(200);

  updateTime();
  GetPrm();
  mp3_init();

  uint8_t lastSel = EEPROM.read(ADDR_RUNSEL);
  RunSel = (lastSel >= 100 && lastSel <= 104) ? lastSel : 1;
  if (RunSel >= 100)
    jumat = (EEPROM.read(ADDR_JUMAT) == 1);

  // Inisialisasi Hardware Display P10 & scanning FreeRTOS / timer
  Disp.begin();
  Disp_init();
  update_All_data();

  Serial.println("[SYSTEM] Sistem JWS ESP32 Berjalan Normal!");
}

// =======================================
// === MAIN LOOP =========================
// =======================================

void loop() {
  serviceBluetooth();
  
  updateTime();
  check_mp3();
  check_azzan();
  DoSwap = false;
  fType(1);
  Disp.clear();
  Timer_Minute(1);

  // List of Display Component Block =========
  if (RunSel == 1)
    dwMrq(drawWelcome(), int(Prm.RT), 2, 1);
  if (RunSel == 2)
    dwMrq(drawDateH(), int(Prm.RT), 2, 2);
  if (RunSel == 3)
    dwMrq(drawDateM(), int(Prm.RT), 2, 3);
  if (RunSel == 4)
    drawSholat(4);
  if (RunSel == 5)
    dwMrq(drawInfo(130), int(Prm.RT), 1, 5);
  if (RunSel == 6)
    drawSholat(6);
  if (RunSel == 7)
    dwMrq(drawInfo(280), int(Prm.RT), 1, 7);
  if (RunSel == 8)
    drawSholat(8);
  if (RunSel == 9)
    dwMrq(drawInfo(430), int(Prm.RT), 1, 9);

  drawOnAzzan(99);
  drawAzzan(100);
  drawIqomah(101);
  if (RunSel == 102)
    dwMrq(drawInfo(580), Prm.RT, 1, 102);  // Message Sholat biasa
  if (RunSel == 103)
    dwMrq(drawInfo(730), Prm.RT, 1, 103);  // Message Sholat jumat
  blinkBlock(104);

  // Display Control Block ===================
  switch (RunFinish) {
    case 1:  setRunSel(2); break;
    case 2:  setRunSel(3); break;
    case 3:  setRunSel(4); break;
    case 4:  setRunSel(5); break;
    case 5:  setRunSel(6); break;
    case 6:  setRunSel(7); break;
    case 7:  setRunSel(8); break;
    case 8:  setRunSel(9); break;
    case 9:  setRunSel(1); break;
    case 98: setRunSel(99); break;
    case 99: setRunSel(100); break;
    case 100:
      if (jumat) {
        setRunSel(103);
        reset_x = 1;
      } else {
        setRunSel(101);
      }
      break;
    case 101:
      setRunSel(102);
      reset_x = 1;
      break;
    case 102:
      setRunSel(104);
      break;
    case 103:
      setRunSel(104);
      break;
    case 104:
      setRunSel(1);
      reset_x = 1;
      break;
    default:
      break;
  }
  RunFinish = 0;

  // Swap Buffer if Change
  if (DoSwap) { Disp.swapBuffers(); }
}
