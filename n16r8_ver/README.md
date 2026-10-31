# MotoMap S3 — ESP32-S3 N16R8 Edition
*Heading-Up Navigation, Offline Maps, Fall Detection & Emergency Zalo SOS*  
*Chỉ đường Heading-Up, Bản đồ Offline, Cảm biến té ngã & Cảnh báo SOS Zalo*

---

## 🌐 Language / Ngôn ngữ
- [English](#-english)
- [Tiếng Việt](#-tiếng-việt)

---

# 🇬🇧 English

Dedicated firmware for **ESP32-S3 DevKitC-1 N16R8** (**16 MB Flash, 8 MB Octal PSRAM**). MotoMap is a standalone motorcycle navigation and rider safety device featuring offline and real-time mapping, fall detection via IMU, and automatic emergency SOS messaging via Zalo Bot.

### ✨ Key Features
1. **Heading-Up Navigation:** Smooth map and route rotation matching vehicle heading. Navigation arrow fixed at 12 o'clock.
2. **Real-time GPS Tracking:** Dynamic updates at 1 Hz – 5 Hz without artificial speed locks.
3. **Double Buffering in 8MB PSRAM:** 400×400 rotating map canvas and 320×240 framebuffer rendered in PSRAM via DMA SPI (zero flicker, zero Flash wear).
4. **Proactive Route Tile Pre-Fetching:** Background task pre-caches tiles along upcoming route segments.
5. **Fall Detection via IMU (MPU-6050):** On-device FreeRTOS task on Core 0 calculates real-time Roll and Pitch angles. If tilt exceeds 45° for 5 seconds, an emergency alert is triggered.
6. **Automated SOS Alerts via Zalo Bot:** Automatically packages latest GPS location, heading, and speed, and sends an alert through a Cloudflare Gateway Worker to emergency Zalo contacts.
7. **Interactive Web Dashboard:** Real-time motorcycle tilt visualizer (SVG), offline map downloader, and Wi-Fi/alert settings.
8. **10s 24 FPS Boot Video Animation:** Fully loaded into Octal PSRAM for stutter-free playback.

---

### 🔌 Pinout Configuration (ESP32-S3 N16R8)

> ⚠️ **CRITICAL HARDWARE NOTE:**  
> On ESP32-S3 N16R8, **GPIO 26 through GPIO 37** are reserved internally for Octal Flash and Octal PSRAM. **DO NOT CONNECT ANY PINS IN THIS RANGE.**

| Peripheral | Pin Label | ESP32-S3 GPIO | Function / Notes |
| :--- | :--- | :--- | :--- |
| **ST7789 Display** | VCC | 3V3 / 5V | Power |
| | GND | GND | Ground |
| | **SCL / SCK** | **GPIO 13** | SPI Clock (FSPI) |
| | **SDA / MOSI** | **GPIO 12** | SPI Data (FSPI) |
| | **RST / RES** | **GPIO 11** | Reset |
| | **DC / A0** | **GPIO 10** | Data / Command |
| | **CS** | **GPIO 9** | Chip Select *(Dedicated to LCD)* |
| **NEO-6M GPS** | VCC | 3V3 / 5V | Power |
| | GND | GND | Ground |
| | TX | **GPIO 17** | GPS TX -> ESP32-S3 RX |
| | RX | **GPIO 18** | GPS RX <- ESP32-S3 TX (Optional) |
| **MPU-6050 IMU** | VCC | 3V3 | Power |
| | GND | GND | Ground |
| | **SDA** | **GPIO 8** | I2C Data |
| | **SCL** | **GPIO 7** | I2C Clock *(Avoid GPIO 9 CS conflict)* |
| **Push Button** | KEY / BOOT | **GPIO 0** | Onboard BOOT Button (Active LOW) |

---

### 🛠️ Build and Flash (PlatformIO)

```bash
cd n16r8_ver

# Build firmware
pio run -e esp32s3_n16r8

# Build filesystem (LittleFS)
pio run -e esp32s3_n16r8 -t buildfs

# Upload firmware and filesystem
pio run -e esp32s3_n16r8 -t upload
pio run -e esp32s3_n16r8 -t uploadfs

# Open Serial Monitor
pio device monitor --baud 115200
```

---

### 📡 HTTP REST API Reference

| Method | Endpoint | Description |
| :--- | :--- | :--- |
| `GET` | `/api/status` | System status, navigation state, GPS snapshot |
| `GET` | `/api/config` | Current configuration |
| `POST` | `/api/config/wifi` | Set Wi-Fi hotspot SSID and password |
| `POST` | `/api/destination` | Set route destination coordinates |
| `POST` | `/api/navigation/stop` | Cancel active route navigation |
| `POST` | `/api/navigation/reroute` | Force recalculate route from current GPS |
| `GET` | `/api/imu` | Real-time roll/pitch, acceleration, gyro, fall status |
| `POST` | `/api/imu/clear-alert` | Manually dismiss fall detection alert |
| `GET` | `/api/alert-config` | Read emergency Zalo alert settings |
| `POST` | `/api/alert-config` | Update Cloudflare Gateway URL, token, Chat IDs |
| `POST` | `/api/alert-test` | Dispatch test SOS alert with current GPS location |

---

# 🇻🇳 Tiếng Việt

Bản nâng cấp firmware chuyên dụng cho bo mạch **ESP32-S3 DevKitC-1 N16R8** (**16 MB Flash, 8 MB Octal PSRAM**). MotoMap là thiết bị dẫn đường độc lập trên xe máy kiêm hệ thống an toàn thông minh: dẫn đường xoay theo hướng xe, bản đồ offline, cảm biến phát hiện té ngã và tự động gửi tin nhắn khẩn cấp qua Zalo Bot.

### ✨ Tính năng nổi bật
1. **Bản đồ xoay theo hướng di chuyển (Heading-Up Navigation):** Bản đồ và tuyến đường OSRM tự động xoay mượt mà theo góc la bàn / GPS. Mũi tên dẫn đường cố định ở góc 12 giờ.
2. **Định vị GPS thời gian thực:** Cập nhật vị trí liên tục ở tần số 1 Hz – 5 Hz, không bị giới hạn vận tốc.
3. **Double Buffering 100% trong 8MB PSRAM:** Canvas bản đồ xoay 400×400 và Framebuffer 320×240 xử lý hoàn toàn trong PSRAM qua DMA SPI (không chớp nháy, không ghi Flash hại chip nhớ).
4. **Tải trước bản đồ theo lộ trình (Tile Pre-Fetching):** Tác vụ nền tự động tải trước các mảnh bản đồ trên đoạn đường sắp đi qua.
5. **Cảm biến phát hiện té ngã (MPU-6050):** Tác vụ FreeRTOS trên Core 0 tính toán góc nghiêng Roll và Pitch. Khi xe nghiêng quá 45° liên tục trong 5 giây, hệ thống tự động xác nhận tai nạn.
6. **Tự động gửi cảnh báo SOS qua Zalo Bot:** Đóng gói toạ độ GPS, vận tốc, hướng di chuyển và gửi qua Cloudflare Gateway Worker tới tài khoản Zalo của người thân.
7. **Web Dashboard trực quan:** Mô phỏng góc nghiêng xe máy thời gian thực (SVG), tải bản đồ offline, cấu hình Wi-Fi và SOS.
8. **Video Animation 10s 24 FPS mượt mà:** Nạp sẵn toàn bộ vào Octal PSRAM cho trải nghiệm chuyển động trơn tru.

---

### 🔌 Sơ đồ đấu nối chân (ESP32-S3 N16R8)

> ⚠️ **LƯU Ý CỰC KỲ QUAN TRỌNG:**  
> Trên ESP32-S3 N16R8, dải chân **GPIO 26 đến GPIO 37** được dùng nội bộ cho Octal Flash và Octal PSRAM. **TUYỆT ĐỐI KHÔNG ĐẤU NỐI VÀO CÁC CHÂN NÀY.**

| Thiết bị ngoại vi | Chân linh kiện | GPIO ESP32-S3 | Chức năng / Ghi chú |
| :--- | :--- | :--- | :--- |
| **Màn hình ST7789** | VCC | 3V3 / 5V | Nguồn màn hình |
| | GND | GND | Mass |
| | **SCL / SCK** | **GPIO 13** | SPI Clock (FSPI) |
| | **SDA / MOSI** | **GPIO 12** | SPI Data (FSPI) |
| | **RST / RES** | **GPIO 11** | Reset màn hình |
| | **DC / A0** | **GPIO 10** | Data / Command |
| | **CS** | **GPIO 9** | Chip Select *(Dành riêng cho LCD)* |
| **Module GPS NEO-6M** | VCC | 3V3 / 5V | Nguồn GPS |
| | GND | GND | Mass |
| | TX | **GPIO 17** | GPS TX -> ESP32-S3 RX |
| | RX | **GPIO 18** | GPS RX <- ESP32-S3 TX (Tùy chọn) |
| **Cảm biến MPU-6050** | VCC | 3V3 | Nguồn cảm biến |
| | GND | GND | Mass |
| | **SDA** | **GPIO 8** | I2C Data |
| | **SCL** | **GPIO 7** | I2C Clock *(Tránh xung đột chân CS 9)* |
| **Nút nhấn vật lý** | KEY / BOOT | **GPIO 0** | Nút BOOT tích hợp trên bo mạch |

---

### 🛠️ Lệnh biên dịch và Nạp Code

```bash
cd n16r8_ver

# 1. Biên dịch firmware
pio run -e esp32s3_n16r8

# 2. Đóng gói phân vùng LittleFS (Web UI & Assets)
pio run -e esp32s3_n16r8 -t buildfs

# 3. Nạp firmware và LittleFS vào mạch
pio run -e esp32s3_n16r8 -t upload
pio run -e esp32s3_n16r8 -t uploadfs

# 4. Mở Serial Monitor theo dõi log
pio device monitor --baud 115200
```
