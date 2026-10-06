/*=============================================
 * PROGMEM DATA
 ==============================================*/
// sholatN 9 x 8
const char static sholatN_E[] PROGMEM = { "IMSAK\0\0\0"
                                          "SUBUH\0\0\0"
                                          "TERBIT\0\0"
                                          "DHUHA\0\0\0"
                                          "DZUHUR\0\0"
                                          "ASHAR\0\0\0"
                                          "MAGHRIB\0"
                                          "ISYA\0\0\0\0"
                                          "JUM'AT\0\0" };
//h_month 12 x 11
const char static h_month_E[] PROGMEM = { "MUHARRAM\0\0\0"
                                          "SHAFAR\0\0\0\0\0"
                                          "RAB.AWAL\0\0\0"
                                          "RAB.AKHIR\0\0"
                                          "JUM.AWAL\0\0\0"
                                          "JUM.AKHIR\0\0"
                                          "RAJAB\0\0\0\0\0\0"
                                          "SYA'BAN\0\0\0\0"
                                          "RAMADHAN\0\0\0"
                                          "SYAWAL\0\0\0\0\0"
                                          "DZULQA'DAH\0"
                                          "DZULHIJJAH\0" };
//m_month 12 x 4
const char static m_month_E[] PROGMEM = { "JAN\0"
                                          "FEB\0"
                                          "MAR\0"
                                          "APR\0"
                                          "MEI\0"
                                          "JUN\0"
                                          "JUL\0"
                                          "AGS\0"
                                          "SEP\0"
                                          "OKT\0"
                                          "NOV\0"
                                          "DES\0" };
//DayName 7 x 7
const char static DayName_E[] PROGMEM = {
  "SENIN\0\0"
  "SELASA\0"
  "RABU\0\0\0"
  "KAMIS\0\0"
  "JUM'AT\0"
  "SABTU\0\0"
  "AHAD\0\0\0"
};
//MT_Name 4 x 10
const char static MT_Name_E[] PROGMEM = { "MASJID\0\0\0\0"
                                          "MUSHOLLA\0\0"
                                          "SURAU\0\0\0\0\0"
                                          "LANGGAR\0\0\0" };

//==============================================
// Drawing Tools================================
//==============================================

// Cache SRAM terkontrol. Lima running text tidak disimpan bersamaan karena
// akan menghabiskan 750 byte; hanya teks layar aktif yang dicache.
static char cachedMasjidName[40];
static char cachedMasjidAddress[50];
static char cachedWelcome[120];
static char cachedRunningText[150];
static uint16_t cachedRunningAddress = 0xFFFF;

void readEepromText(uint16_t address, char* destination, size_t destinationSize) {
  if (destinationSize == 0) return;

  size_t i = 0;
  for (; i < destinationSize - 1; i++) {
    const char value = (char)EEPROM.read(address + i);
    destination[i] = value;
    if (value == '\0') break;
  }
  destination[(i < destinationSize) ? i : destinationSize - 1] = '\0';
}

void loadDisplayCache() {
  char masjidType[11];
  uint8_t typeIndex = Prm.MT;
  if (typeIndex < 1 || typeIndex > 4) typeIndex = 1;

  memcpy_P(masjidType, MT_Name_E + ((typeIndex - 1U) * 10U), 10);
  masjidType[10] = '\0';

  readEepromText(40, cachedMasjidName, sizeof(cachedMasjidName));
  readEepromText(80, cachedMasjidAddress, sizeof(cachedMasjidAddress));
  strcpy_P(cachedWelcome, PSTR("SELAMAT DATANG DI "));
  strcat(cachedWelcome, masjidType);
  strcat_P(cachedWelcome, PSTR(" "));
  strcat(cachedWelcome, cachedMasjidName);
  strcat_P(cachedWelcome, PSTR(" "));
  strcat(cachedWelcome, cachedMasjidAddress);

  cachedRunningAddress = 0xFFFF;
  cachedRunningText[0] = '\0';
}

char* sholatN(int number)  // get sholat name from PROGMEM
{
  static char locBuff[9];
  if (number < 0 || number > 8) number = 0;
  int locLen = number * 8;
  memcpy_P(locBuff, sholatN_E + locLen, 8);
  locBuff[8] = '\0';
  return locBuff;
}

char* DayName(int number)  // get Day Name from PROGMEM
{
  static char locBuff[8];
  if (number < 1 || number > 7) number = 1;
  int locLen = (number - 1) * 7;
  memcpy_P(locBuff, DayName_E + locLen, 7);
  locBuff[7] = '\0';
  return locBuff;
}

char* drawDateM() {
  static char out[16];
  static char locBuff[5];
  uint8_t m = now.month();
  if (m < 1 || m > 12) m = 1;
  int locLen = (m - 1) * 4;
  memcpy_P(locBuff, m_month_E + locLen, 4);
  locBuff[4] = '\0';

  uint8_t d = now.day();
  out[0] = (d / 10) + '0';
  out[1] = (d % 10) + '0';
  out[2] = '-';
  strcpy(out + 3, locBuff);
  uint8_t len = strlen(out);
  out[len++] = '-';
  utoa(now.year(), out + len, 10);
  len = strlen(out);
  out[len++] = ' ';
  out[len++] = 'M';
  out[len] = '\0';

  return out;
}

char* drawDateH() {
  char locBuff[11];
  static char out[32];
  uint8_t hm = nowH.hM;
  if (hm < 1 || hm > 12) hm = 1;
  int locLen = (hm - 1) * 11;
  memcpy_P(locBuff, h_month_E + locLen, 11);
  locBuff[10] = '\0';

  strcpy(out, DayName(daynow));
  uint8_t len = strlen(out);
  out[len++] = ',';
  out[len++] = ' ';
  out[len++] = (nowH.hD / 10) + '0';
  out[len++] = (nowH.hD % 10) + '0';
  out[len++] = ' ';
  out[len] = '\0';
  strcat(out, locBuff);
  len = strlen(out);
  out[len++] = ' ';
  utoa(nowH.hY, out + len, 10);
  len = strlen(out);
  out[len++] = ' ';
  out[len++] = 'H';
  out[len] = '\0';

  return out;
}

char* drawWelcome() {
  return cachedWelcome;
}

char* drawInfo(uint16_t addr) {
  if (cachedRunningAddress != addr) {
    readEepromText(addr, cachedRunningText, sizeof(cachedRunningText));
    cachedRunningAddress = addr;
  }
  return cachedRunningText;
}