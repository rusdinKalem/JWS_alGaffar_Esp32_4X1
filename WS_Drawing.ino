// =========================================
// Drawing Content Block====================
// =========================================
void formatDuaAngka(int nilai, char* hasil) {
  hasil[0] = (nilai / 10) + '0';
  hasil[1] = (nilai % 10) + '0';
  hasil[2] = '\0';
}

static const uint8_t satu[] PROGMEM = {
  16,
  14,
  0x18,
  0x08,
  0x0C,
  0x14,
  0x06,
  0x1C,
  0x06,
  0x70,
  0x06,
  0xC0,
  0x06,
  0x80,
  0x06,
  0xFF,
  0x06,
  0x1E,
  0x1B,
  0xF8,
  0xB1,
  0x80,
  0xE0,
  0x6C,
  0x00,
  0x7E,
  0x00,
  0x12,
  0x00,
  0x0C,
};
static const uint8_t dua[] PROGMEM = {
  16,
  15,
  0x00,
  0x84,
  0x05,
  0x84,
  0x07,
  0x04,
  0x00,
  0x04,
  0x00,
  0x04,
  0x00,
  0x24,
  0x00,
  0xA6,
  0x08,
  0xB2,
  0x08,
  0xB2,
  0x18,
  0x92,
  0x38,
  0xD2,
  0x68,
  0xF2,
  0x7D,
  0xF0,
  0xF7,
  0x30,
  0x07,
  0x00,
};

// =========================================
// PROGMEM String Pool (label statis display)
// =========================================
static const char P_TIBA[] PROGMEM = "TIBA";
static const char P_WAKTU[] PROGMEM = "WAKTU";
static const char P_ADZAN[] PROGMEM = "ADZAN";
static const char P_ACARA[] PROGMEM = "ACARA";
static const char P_JUMATAN[] PROGMEM = "JUM'AT";
static const char P_IQOMAH[] PROGMEM = "IQOMAH";

// Helper: salin PROGMEM -> buffer RAM lalu gambar (DMD membaca dari RAM)
void dwText_P(int x, int y, PGM_P p) {
  static char buf[16];
  strcpy_P(buf, p);
  Disp.drawText(x, y, buf);
}

void dwCtr_P(int x, int y, PGM_P p) {
  static char buf[16];
  strcpy_P(buf, p);
  dwCtr(x, y, buf);
}
// =========================================
// Drawing Content Azzan , Iqomah & Blink ==
// =========================================
void drawOnAzzan(int DrawAdd) {
  // check RunSelector
  if (!dwDo(DrawAdd)) return;
  uint8_t ct_kedip = 20;  //harus angka genap
  static uint8_t ct;
  static uint16_t lsRn;
  uint16_t Tmr = millis();

  if ((Tmr - lsRn) > 500 and ct <= ct_kedip) {
    lsRn = Tmr;
    if ((ct % 2) == 0) {
      fType(1);
      Disp.drawRect(0, 0, 31, 15);
      Disp.drawRect(2, 2, 29, 13);
      dwText_P(4, 5, P_TIBA);
      Disp.drawRect(33, 3, 126, 12);
      fType(2);
      dwCtr_P(-12, 0, P_WAKTU);
      if (jumat) {
        dwCtr(75, 7, sholatN(8));
      } else {
        dwCtr(75, 7, sholatN(SholatNow));
      }
      { Buzzer(1); }
    } else {
      Buzzer(0);
    }
    DoSwap = true;
    ct++;
  }

  if (ct > ct_kedip) {
    dwDone(DrawAdd);
    ct = 0;
    Buzzer(0);
  }
}

//================================================================

void drawAzzan(int DrawAdd) {
  // check RunSelector
  if (!dwDo(DrawAdd)) return;
  uint16_t az = Prm.AD, in = Prm.IN;
  static int ct;
  static uint16_t lsRn;
  uint16_t Tmr = millis();
  int ct_limit, mnt, scd;
  char BuffMnt[5];
  char BuffScd[5];

  if (jumat) {
    ct_limit = in * 60;
  } else {
    ct_limit = az * 60;
  }

  if ((Tmr - lsRn) > 1000 and ct <= ct_limit) {
    lsRn = Tmr;
    
    mnt = (ct_limit - ct) / 60;
    scd = (ct_limit - ct) % 60;
    formatDuaAngka(mnt, BuffMnt);
    formatDuaAngka(scd, BuffScd);


    if (mnt > 0) {
      Disp.setFont(BigNumber);
      Disp.drawText(1, 0, BuffMnt);
      Disp.drawText(18, 0, BuffScd);
      Disp.drawFilledRect(15, 4, 16, 6);
      Disp.drawFilledRect(15, 10, 16, 12);
    }  // MENAMPILKAN MENIT
    else {
      Disp.setFont(BigNumber);
      Disp.drawText(10, 0, BuffScd);
    }  // MENAMPILKAN DETIK

    Disp.drawRect(33, 3, 126, 12);
    fType(2);
    if (jumat) {
      dwCtr_P(-12, 0, P_ACARA);
      dwCtr_P(75, 7, P_JUMATAN);
    } else {
      dwCtr_P(-12, 0, P_ADZAN);
      dwCtr(75, 7, sholatN(SholatNow));
    }
    if (ct > (ct_limit - 5))
      Buzzer(1);
    DoSwap = true;
    ct++;
  }

  if (ct > ct_limit) {
    dwDone(DrawAdd);
    ct = 0;
  }
}

//================================================================

void drawIqomah(int DrawAdd) {
  // check RunSelector
  if (!dwDo(DrawAdd)) return;

  static uint16_t lsRn;
  uint16_t Tmr = millis();
  static int ct;
  int mnt, scd, cn_l;
  char BuffMnt[5];
  char BuffScd[5];


  cn_l = (Iqomah[SholatNow] * 60);


  Disp.drawRect(33, 2, 126, 13);

  if ((Tmr - lsRn) > 1000 and ct <= cn_l) {
    lsRn = Tmr;
    mnt = (cn_l - ct) / 60;
    scd = (cn_l - ct) % 60;
    formatDuaAngka(mnt, BuffMnt);
    formatDuaAngka(scd, BuffScd);

    if (mnt > 0) {
      Disp.setFont(BigNumber);
      Disp.drawText(1, 0, BuffMnt);
      Disp.drawText(18, 0, BuffScd);
      Disp.drawFilledRect(15, 4, 16, 6);
      Disp.drawFilledRect(15, 10, 16, 12);
    }  // MENAMPILKAN MENIT
    else {
      Disp.setFont(BigNumber);
      Disp.drawText(10, 0, BuffScd);
    }  // MENAMPILKAN DETIK
    fType(2);

    dwCtr_P(35, 4 , P_IQOMAH);
    if (ct > (cn_l - 11))
      Buzzer(1);
    ct++;
    DoSwap = true;
  }
  if (ct > cn_l) {
    dwDone(DrawAdd);
    ct = 0;
    Buzzer(0);
  }
}

//=============================================================================================================

void blinkBlock(int DrawAdd) {
  // check RunSelector
  if (!dwDo(DrawAdd)) return;

  static uint16_t lsRn;
  static uint16_t ct, ct_l;
  uint16_t Tmr = millis();

  if (jumat) {
    ct_l = (Prm.JM + Prm.SO) * 60;
  } else {
    ct_l = Prm.SO * 60;
  }

  if ((Tmr - lsRn) > 1000) {
    lsRn = Tmr;

    if ((ct % 2) == 0) { Disp.drawFilledRect(DWidth - 3, DHeight - 3, DWidth - 2, DHeight - 2); }
    DoSwap = true;
    ct++;
  }

  if (ct > ct_l) {
    dwDone(DrawAdd);
    azzan = false;
    jumat = false;
    ct = 0;
  }
}

// =========================================
// Drawing Content Clock & JWS =============
// =========================================

void drawSholat_S(int sNum, int c)  // Box Sholah Time
{
  char BuffTime[10];
  char BuffShol[7];
  float stime = sholatT[sNum];
  uint8_t shour = floor(stime);
  uint8_t sminute = floor((stime - (float)shour) * 60);
  formatDuaAngka(shour, BuffTime);
  BuffTime[2] = ':';
  formatDuaAngka(sminute, BuffTime + 3);
  Disp.drawRect(c + 1, 3, 125, 12);
  fType(2);
  dwCtr(c - 40, 0, sholatN(sNum));
  fType(2);
  dwCtr(c + 50, 7, BuffTime);
  DoSwap = true;
}

//================================================================================

void drawSholat(int DrawAdd) {
  // check RunSelector
  //    int DrawAdd = 0b0000000000000100;
  if (!dwDo(DrawAdd)) return;

  static uint8_t x;
  static uint8_t s;  // 0=in, 1=out
  static uint8_t sNum;
  static uint16_t lsRn;
  uint16_t Tmr = millis();
  uint8_t c = 33;
  uint8_t first_sNum = 0;
  int DrawWd = DWidth - c;

  if ((Tmr - lsRn) > 10) {
    if (s == 0 and x < (DrawWd / 2)) {
      x++;
      lsRn = Tmr;
    }
    if (s == 1 and x > 0) {
      x--;
      lsRn = Tmr;
    }
  }

  if ((Tmr - lsRn) > 2000 and x == (DrawWd / 2)) { s = 1; }
  if (x == 0 and s == 1) {
    if (sNum < 7) {
      sNum++;
    } else {
      dwDone(DrawAdd);
      sNum = 0;
    }
    s = 0;
  }

  if (Prm.SI == 0) {
    first_sNum = 1;
  } else {
    first_sNum = 0;
  }
  if (Prm.SI == 0 and sNum == 0) { sNum = 1; }
  if (Prm.ST == 0 and sNum == 2) { sNum = 3; }
  if (Prm.SU == 0 and sNum == 3) { sNum = 4; }


  if ((((sNum == first_sNum) and s == 0) or ((sNum == 7) and s == 1))
      and x <= 20) {
    drawSmallTS(int(x / 2));
  } else {
    drawSmallTS(10);
  }
  drawSholat_S(sNum, c);

  Disp.drawFilledRect(c, 0, c + DrawWd / 2 - x, 16, 0);
  Disp.drawFilledRect(DrawWd / 2 + x + c, 0, 126, 16, 0);
}

/**************************************************************************************/

void drawSmallTS(int x)  //Draw Jam dan Menit
{
  char BuffH[3];
  char BuffM[3];
  static uint16_t lsRn;
  uint16_t Tmr = millis();
  formatDuaAngka(now.hour(), BuffH);
  formatDuaAngka(now.minute(), BuffM);
  Disp.setFont(BigNumber);
  Disp.drawFilledRect(32, 16, 0, 0, 0);
  Disp.drawText((x - 9) + 0, 0, BuffH);
  Disp.drawText((x - 9) + 17, 0, BuffM);

  if (Tmr - lsRn < 500) {

    Disp.drawRect((x - 7) + 9 + 4, 3, (x - 10) + 10 + 5, 5, 1);
    Disp.drawRect((x - 7) + 9 + 4, 10, (x - 10) + 10 + 5, 12, 1);
  }

  if (Tmr - lsRn > 1000) lsRn = Tmr;
  DoSwap = true;
}

// =========================================
// Drawing Content Running Text ============
// =========================================

void dwMrq(const char* msg, int Speed, int dDT, int DrawAdd) {
  // check RunSelector
  static uint16_t x;
  if (!dwDo(DrawAdd)) return;
  if (reset_x != 0) {
    x = 0;
    reset_x = 0;
  }


  static uint16_t lsRn;
  int fullScroll = Disp.textWidth(msg) + DWidth;
  uint16_t Tmr = millis();
  if ((Tmr - lsRn) > Speed) {
    lsRn = Tmr;
    if (x < fullScroll) {
      ++x;
    } else {
      dwDone(DrawAdd);
      x = 32;
      return;
    }
    if (dDT == 1) {
      fType(1);
      Disp.drawText(DWidth - x, 5, msg);
      Disp.drawFilledRect(32, 15, 0, 0, 0);
      drawSmallTS(10);
      Disp.drawLine(33, 1, 126, 1);
      Disp.drawLine(33, 14, 126, 14);
      fType(1);
    } else if (dDT == 2) {
      fType(1);
      Disp.drawText(DWidth - x, 5, msg);
      Disp.drawFilledRect(48, 14, 0, 0, 0);
      drawSmallTS(10);
      Disp.drawBitmap(32, 1, satu);
      Disp.drawBitmap(112, 0, dua);
      Disp.drawLine(49, 1, 110, 1);
      Disp.drawLine(49, 14, 110, 14);

    } else {
      fType(1);
      Disp.drawFilledRect(33, 14, 0, 0, 0);
      Disp.drawLine(33, 1, 126, 1);
      Disp.drawLine(33, 14, 126, 14);
      Disp.drawText(DWidth - x, 5, msg);
    }
    DoSwap = true;
    drawSmallTS(10);
  }
}

// =========================================
// Drawing Tools============================
// =========================================
boolean dwDo(int DrawAdd) {
  if (RunSel == DrawAdd) {
    return true;
  } else return false;
}

void dwDone(int DrawAdd) {
  RunFinish = DrawAdd;
  RunSel = 0;
}

void dwCtr(int x, int y, const char* Msg) {
  int tw = Disp.textWidth(Msg);
  int th = Disp.textHeight();
  int c = int((DWidth - x - tw) / 2);
  Disp.drawFilledRect(x + c - 1, y, x + tw + c, y + th, 0);
  Disp.drawText(x + c, y, Msg);
}

void Buzzer(uint8_t state) {
  if (state == 1 and Prm.BZ == 1) {
    tone(BUZZ, 500, 400);
  } else {
    noTone(BUZZ);
  }
}

void fType(int x) {
  if (x == 0) Disp.setFont(Font4x6);
  else if (x == 1) Disp.setFont(System5x7);
  else Disp.setFont(Font6x7);
}
