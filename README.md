# JWS alGAFFAR Panel P10 HUB12 (4x1 Panel, 128x16 Piksel) - ESP32

Firmware Jam Waktu Sholat (JWS) otomatis berbasis **ESP32 WROOM-32** untuk mengendalikan **Panel LED P10 Single Color (HUB12)** berukuran **4x1 panel (128x16 piksel)**. Firmware ini merupakan migrasi dan peningkatan dari versi Arduino Uno (DMD3) dengan performa bebas flicker (*double buffered*), modul Bluetooth internal, pemutar audio MP3 DFPlayer Mini mandiri, serta file desain PCB kustom siap cetak (Gerber) untuk pemesanan di JLCPCB.

---

## 🌟 Fitur Utama

- **Layar Panel P10 128 x 16 Piksel Bebas Kedip (*Flicker-Free*)**:
  - Menggunakan driver bawaan performa tinggi **`DMD3_ESP32`** dengan hardware timer ESP32 (`esp_timer`) berfrekuensi scanning presisi 2.0 ms.
  - Mendukung penuh **Double Buffering** (`swapBuffers`) sehingga pergerakan jam dan animasi teks berjalan sangat halus tanpa kedip (*tearing*).
  - Tampilan jam digital besar (`BigNumber`), kalender Masehi & Hijriah otomatis, nama waktu sholat, dan jadwal sholat bergantian.
  - Animasi running text hadits, jadwal kegiatan, dan pengumuman masjid yang mulus.
- **Hisab Waktu Sholat Akurat**:
  - Algoritma hisab waktu sholat berdasarkan koordinat latitude, longitude, ketinggian, dan zona waktu.
  - Pengaman Ihtiyati dan koreksi waktu per jadwal sholat (Subuh, Terbit, Dzuhur, Ashar, Maghrib, Isya).
- **Fase Masuk Waktu Sholat**:
  - **Fase Adzan**: Tampilan berkedip, penanda waktu adzan masuk, dan bunyi beep buzzer.
  - **Fase Iqomah**: Hitung mundur menit dan detik iqomah disertai tanda bunyi beep penanda 10 detik terakhir.
  - **Fase Sholat (Blackout)**: Pesan *"LURUSKAN SHAF"* diikuti pemadaman layar sementara selama sholat berjamaah berlangsung agar tidak menyilaukan jamaah.
- **Audio DFPlayer Mini (Tartil & Tarhim Otomatis)**:
  - Berjalan pada Hardware Serial2 (`SerialMP3`) mandiri (GPIO 16 RX & GPIO 17 TX) dengan baudrate 9600 bps.
  - Otomatis memutar murottal/tartil Al-Qur'an (Folder 01) sebelum waktu sholat sesuai durasi yang disetel.
  - Otomatis memutar tarhim/sholawat (Folder 02) menjelang adzan.
  - Otomatis mematikan suara saat adzan atau sholat dimulai.
- **Konektivitas Bluetooth Classic Internal (SPP)**:
  - Menggunakan Bluetooth Classic bawaan ESP32 (Nama Bluetooth: **`JWS-alGaffar`**).
  - Tidak memerlukan modul tambahan seperti HC-05 atau HC-06.
  - **100% Kompatibel dengan Aplikasi Android alGaffar** untuk pengaturan teks, nama masjid, koreksi waktu, koordinat GPS, dan jadwal sholat.
- **Penyimpanan Flash NVS (EEPROM)**:
  - Seluruh parameter tersimpan aman di Flash NVS ESP32 dan tidak hilang saat listrik padam.

---

## 🔌 Pemetaan Pin Hardware (ESP32 WROOM-32)

Susunan pin dirancang agar jalur bus VSPI P10, I2C RTC DS3231, dan Serial MP3 tidak saling bertabrakan:

| Perangkat / Jalur | Pin ESP32 | Keterangan |
| :--- | :--- | :--- |
| **P10 HUB12 OE** | `GPIO 15` | Output Enable (Kontrol Kecerahan) |
| **P10 HUB12 A** | `GPIO 19` | Baris Scan A |
| **P10 HUB12 B** | `GPIO 27` | Baris Scan B |
| **P10 HUB12 CLK** | `GPIO 18` | SPI Clock (VSPI SCK) |
| **P10 HUB12 LAT / SCLK** | `GPIO 2` | Shift Register Latch |
| **P10 HUB12 DATA / R** | `GPIO 23` | SPI Data (VSPI MOSI) |
| **RTC DS3231 SDA** | `GPIO 21` | I2C Data Default ESP32 |
| **RTC DS3231 SCL** | `GPIO 22` | I2C Clock Default ESP32 |
| **DFPlayer Mini RX** | `GPIO 17 (TX2)` | UART2 TX ESP32 (Pasang Resistor 1kΩ ke RX DFPlayer) |
| **DFPlayer Mini TX** | `GPIO 16 (RX2)` | UART2 RX ESP32 |
| **Buzzer Aktif 5V** | `GPIO 4` | Driver Transistor Buzzer NPN (S8050) |

---

## 🛠️ Persyaratan Library & Konfigurasi Arduino IDE

### Library yang Dibutuhkan
1. Core ESP32 Arduino Board Package (versi 2.0.x atau 3.0.x)
2. `BluetoothSerial` (Bawaan ESP32 Core)
3. `Wire` (Bawaan Arduino)
4. `EEPROM` (Bawaan ESP32 Core)

*Driver `DMD3_ESP32` beserta engine font dan grafis `Bitmap` sudah terintegrasi langsung di dalam folder proyek ini.*

### Pengaturan Board di Arduino IDE
- **Board**: `ESP32 Dev Module`
- **Partition Scheme**: `Huge APP (3MB No OTA/1MB SPIFFS)` atau `Default 4MB with spiffs`
- **Upload Speed**: `921600` (atau `115200`)
- **CPU Frequency**: `240MHz (WiFi/BT)`
- **Flash Frequency**: `80MHz`
- **Port**: Pilih COM Port ESP32 Anda

---

## 📦 File Fabrikasi PCB (Gerber Package)

Project ini dilengkapi file fabrikasi PCB RS-274X + Excellon drill file siap cetak di [JLCPCB](https://jlcpcb.com):
- **Ukuran PCB**: 100.0 mm × 75.0 mm (2 Layer FR-4)
- **Silkscreen Atas**: `JWS alGAFFAR` & `ESP32 P10 HUB12 + MP3`
- **Silkscreen Bawah**: `JWS alGAFFAR - ESP32` & `DESIGNED BY Roesch`
- **File Arsip Siap Order**:
  - `Gerber_JWS_alGaffar_ESP32_4X1.zip`
  - `Gerber_JWS_alGAFFAR_ESP32.zip`

---

## 📱 Penggunaan Aplikasi Android (alGaffar)

1. Nyalakan perangkat JWS alGAFFAR.
2. Buka menu Bluetooth di HP Android, cari perangkat bernama **`JWS-alGaffar`**, lalu lakukan proses pairing (PIN standar: `1234` atau tanpa PIN).
3. Buka aplikasi **alGaffar**, pilih koneksi Bluetooth ke perangkat.
4. Anda dapat langsung mengirim data waktu, koordinat lintang/bujur, nama masjid, durasi iqomah, serta pesan teks berjalan.
