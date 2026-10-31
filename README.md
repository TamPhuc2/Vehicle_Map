# MotoMap — Smart Motorcycle Navigation & Safety System
*Hệ thống dẫn đường thông minh & Cảnh báo an toàn cho xe máy*

---

## 🌐 Language / Ngôn ngữ
- [English](#-english)
- [Tiếng Việt](#-tiếng-việt)

---

# 🇬🇧 English

MotoMap is an open-source, standalone motorcycle navigation and rider safety device powered by ESP32 microcontrollers. Using an onboard GPS module and high-visibility SPI display, MotoMap delivers turn-by-turn navigation without requiring continuous phone screen operation.

### 🚀 Key Hardware Editions
1. **[ESP32-S3 N16R8 Edition (`n16r8_ver`)](n16r8_ver/README.md) *(Recommended / Active)***:
   - **Hardware:** ESP32-S3 DevKitC-1 (16 MB Flash, 8 MB Octal PSRAM).
   - **Display:** 2.0" / 1.69" ST7789 320×240 IPS display.
   - **Heading-Up Navigation:** Real-time rotating map and route aligned with vehicle heading.
   - **Double-Buffered in PSRAM:** 400×400 canvas rendered entirely in Octal PSRAM (zero flicker, zero Flash wear).
   - **Offline Maps:** 13.25 MB LittleFS map tile storage with 3-tier memory caching.
   - **Fall & Crash Detection:** Real-time IMU MPU-6050 monitoring on FreeRTOS Core 0.
   - **Automated Emergency SOS:** Auto-triggers location and crash alerts via Cloudflare Gateway to Zalo Bot.
   - **Video Animations:** Smooth 24 FPS 10-second boot animations loaded into PSRAM.

2. **[ESP32 Classic Edition (`moto_map`)](moto_map/README.md)**:
   - **Hardware:** ESP32-WROOM-32 / DevKit (4 MB Flash, No PSRAM).
   - **Display:** GMT147SPI ST7789 172×320 display.
   - **Navigation:** Lightweight turn-by-turn arrows and distance indicators.

### 📱 Smartphone Companion & Web Portal
- **Web Portal:** Connect your smartphone to the device's hotspot or Wi-Fi to select destinations, download offline map areas, monitor real-time vehicle tilt, and configure emergency contacts.
- **iOS Companion App:** Native Swift application located in [`ios/MotoMap`](ios/MotoMap/README.md).

---

# 🇻🇳 Tiếng Việt

MotoMap là thiết bị dẫn đường độc lập và cảnh báo an toàn cho xe máy trên nền tảng vi điều khiển ESP32. Kết hợp module định vị GPS và màn hình màu SPI độ sáng cao, MotoMap mang đến trải nghiệm dẫn đường từng khúc cua mà không cần bật sáng màn hình điện thoại liên tục.

### 🚀 Các phiên bản phần cứng
1. **[Phiên bản ESP32-S3 N16R8 (`n16r8_ver`)](n16r8_ver/README.md) *(Khuyên dùng / Đang phát triển)***:
   - **Phần cứng:** ESP32-S3 DevKitC-1 (16 MB Flash, 8 MB Octal PSRAM).
   - **Màn hình:** ST7789 320×240 IPS (2.0" hoặc 1.69").
   - **Bản đồ xoay theo hướng xe (Heading-Up):** Bản đồ và tuyến đường xoay mượt mà theo góc la bàn / GPS.
   - **Double-Buffering trong PSRAM:** Canvas 400×400 xử lý hoàn toàn trong 8MB PSRAM (không giật hình, không hại bộ nhớ Flash).
   - **Bản đồ Offline:** Phân vùng LittleFS 13.25 MB với bộ đệm 3 tầng.
   - **Phát hiện té ngã (IMU MPU-6050):** Tác vụ FreeRTOS chạy độc lập trên Core 0.
   - **Cảnh báo cứu hộ SOS tự động:** Tự động gửi định vị và thông báo ngã tới Zalo Bot người thân qua Cloudflare Gateway.
   - **Video Animation:** Phát video 24 FPS mượt mà trực tiếp từ PSRAM.

2. **[Phiên bản ESP32 Classic (`moto_map`)](moto_map/README.md)**:
   - **Phần cứng:** ESP32 DevKit / WROOM-32 (4 MB Flash, không PSRAM).
   - **Màn hình:** GMT147SPI ST7789 172×320.
   - **Dẫn đường:** Mũi tên chỉ hướng tinh gọn và thanh cự ly rẽ.

### 📱 Giao diện Web Portal & Ứng dụng điện thoại
- **Web Portal:** Mở trình duyệt trên điện thoại kết nối vào Wi-Fi của thiết bị để tìm kiếm điểm đến, tải bản đồ offline, theo dõi góc nghiêng xe và cài đặt số Zalo cứu hộ.
- **Ứng dụng iOS:** Ứng dụng Swift tương thích trong thư mục [`ios/MotoMap`](ios/MotoMap/README.md).
