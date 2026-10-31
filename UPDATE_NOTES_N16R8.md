# MIGRATION & TRANSITION NOTES / TÀI LIỆU CHUYỂN GIAO THAY ĐỔI
> **Target Branch: `n16r8_ver` (ESP32-S3 N16R8)**  
> *Notice: This document provides clear technical details for all team members comparing changes against commit `354215f`. This file is temporary and will be removed upon final release.*  
> *Lưu ý: Tài liệu tổng hợp kỹ thuật tạm thời giải trình các thay đổi so với commit `354215f`. Sẽ được gỡ bỏ khi hoàn thiện dự án.*

---

## 🌐 Quick Navigation / Mục lục
- [English Documentation](#-english-version)
- [Tài liệu Tiếng Việt](#-tiếng-việt-version)

---

# 🇬🇧 English Version

## 1. Summary of Changes (vs. Commit `354215f`)
Commit `354215f` introduced the 13.25MB LittleFS offline map partition and 3-tier memory cache.  
This update integrates two core safety systems:
1. **Emergency SOS Alert via Zalo Bot (Cloudflare Gateway Worker)**.
2. **On-Device Fall & Crash Detection via IMU MPU-6050**.

All code has been restructured cleanly using FreeRTOS tasks pinned to Core 0 (preventing display/GPS rendering lag on Core 1).

---

## 2. Integrated Modules & Architecture

### 2.1. IMU Fall Detection (`ImuManager`)
* **Hardware:** MPU-6050 6-DoF accelerometer & gyroscope via I2C (`0x68`).
* **Task Allocation:** Dedicated FreeRTOS task running every 200ms on **Core 0**.
* **On-Device Detection Algorithm:**
  - Real-time Roll and Pitch computation:
    $$\text{Roll} = \text{atan2}(a_y, a_z) \times \frac{180}{\pi}$$
    $$\text{Pitch} = \text{atan2}(-a_x, \sqrt{a_y^2 + a_z^2}) \times \frac{180}{\pi}$$
  - **Fall Trigger:** Sustained tilt $|\text{Roll}| > 45^\circ$ or $|\text{Pitch}| > 45^\circ$ for **5.0 seconds**.
  - **Automated SOS Dispatch:** Captures current GPS snapshot (Lat, Lon, Speed, Course) from `GpsManager` and enqueues an `AlertEvent` into `AlertManager`.
  - **Recovery / Dismissal:** Automatically resets when vehicle is upright ($< 40^\circ$) for 2.0s, or when manually cleared via the Web Portal.

### 2.2. Emergency Zalo SOS (`AlertManager`)
* **Flow:** `ESP32-S3` $\xrightarrow{\text{HTTPS POST}}$ `Cloudflare Worker Gateway` $\xrightarrow{\text{Zalo API}}$ `Target Zalo Users`.
* **Worker Source:** `n16r8_ver/tools/cloudflare_alert_worker/`. Supports Bearer authentication and dynamic Chat IDs.
* **Persistent NVS:** Stores configuration in `"motomap"` namespace: `alrt_en`, `alrt_url`, `alrt_key`, `alrt_chats`.

### 2.3. Web Dashboard Additions
* **Real-time Tilt Cards:** Interactive animated SVGs representing motorcycle front roll and side pitch.
* **Telemetry Data:** Total acceleration magnitude ($m/s^2$), Angular velocity ($rad/s$), IMU chip temperature ($^\circ\text{C}$).
* **Fall Alert Banner:** High-visibility alert with manual "Clear Alert" button.

---

## 3. Hardware Conflict Resolution

> [!WARNING]
> **GPIO 9 Conflict Resolved:**
> - Original code proposed GPIO 9 for `I2C_SCL`. However, **GPIO 9** is dedicated to **`PIN_ST7789_CS`** (SPI LCD Chip Select).
> - **Current Mapping in `PinConfig.h`:**
>   - **I2C SDA:** **GPIO 8**
>   - **I2C SCL:** **GPIO 7** *(or override with `-DPIN_I2C_SCL=...`)*
>   - *Reminder:* GPIO 26–37 are reserved for Octal Flash & PSRAM.

---

## 4. File Changes Overview

| Path | Status | Purpose |
| :--- | :---: | :--- |
| `n16r8_ver/include/ImuManager.h` | **New** | `ImuData` struct and `ImuManager` class declaration |
| `n16r8_ver/src/ImuManager.cpp` | **New** | Core 0 FreeRTOS task, MPU-6050 sampling, fall detection |
| `n16r8_ver/include/AlertManager.h` | **New** | Queue and task declaration for asynchronous SOS sending |
| `n16r8_ver/src/AlertManager.cpp` | **New** | Secure HTTPS POST to Cloudflare Gateway |
| `n16r8_ver/tools/cloudflare_alert_worker/` | **New** | Serverless relay to Zalo Bot API |
| `n16r8_ver/include/PinConfig.h` | Modified | Added `kI2cSdaPin` (8) and `kI2cSclPin` (7) |
| `n16r8_ver/include/MotoMapTypes.h` | Modified | Added `AlertConfig` and `AlertEvent` |
| `n16r8_ver/include/ConnectionManager.h` | Modified | Added `/api/imu`, `/api/imu/clear-alert`, `/api/alert-*` |
| `n16r8_ver/src/ConnectionManager.cpp` | Modified | API handlers and JSON serialization |
| `n16r8_ver/src/main.cpp` | Modified | Initialized `ImuManager` and `AlertManager` |
| `n16r8_ver/data/index.html` | Modified | Added IMU tilt cards and emergency config UI |
| `n16r8_ver/data/app.js` | Modified | 500ms IMU polling loop and user interactions |
| `n16r8_ver/data/styles.css` | Modified | Styles for tilt SVGs and red alert banner |
| `.gitignore` | Modified | Excluded `.pio`, `.vscode`, `node_modules`, `.wrangler` |

---

# 🇻🇳 Tiếng Việt Version

## 1. Tổng quan thay đổi (So với Commit `354215f`)
Commit `354215f` đã mở rộng phân vùng bản đồ offline LittleFS lên 13.25MB và bộ nhớ đệm 3 tầng.  
Bản cập nhật này tích hợp 2 hệ thống an toàn cốt lõi:
1. **Cảnh báo cứu hộ SOS qua Zalo Bot (Cloudflare Gateway Worker)**.
2. **Cảm biến đo góc nghiêng & tự động phát hiện té ngã (IMU MPU-6050)**.

Toàn bộ code được viết chuẩn FreeRTOS chạy độc lập trên Core 0, không gây giật lag màn hình LCD và định vị GPS trên Core 1.

---

## 2. Chi tiết kỹ thuật các module

### 2.1. Cảm biến góc nghiêng & Té ngã (`ImuManager`)
* **Phần cứng:** Cảm biến 6 trục **MPU-6050** giao tiếp I2C (`0x68`).
* **Phân luồng:** Chạy FreeRTOS task chu kỳ 200ms trên **Core 0**.
* **Thuật toán nhận diện té ngã on-device:**
  - Tính góc nghiêng Roll và Pitch thời gian thực.
  - Khi $|\text{Roll}| > 45^\circ$ hoặc $|\text{Pitch}| > 45^\circ$ liên tục trong **5.0 giây**, ESP32 tự động xác định xe bị ngã.
  - Tự động trích xuất toạ độ GPS (Vĩ độ, Kinh độ, Tốc độ, Hướng) từ `GpsManager` và kích hoạt `AlertManager` gửi tin nhắn SOS đến Zalo người thân.
  - Tự động hạ cờ cảnh báo khi xe được dựng thẳng ($< 40^\circ$) trong 2.0s hoặc khi bấm nút "Xóa cảnh báo" trên Web.

### 2.2. Cảnh báo SOS qua Zalo Bot (`AlertManager`)
* **Luồng truyền:** `ESP32-S3` $\xrightarrow{\text{HTTPS POST}}$ `Cloudflare Worker Gateway` $\xrightarrow{\text{Zalo API}}$ `Zalo người thân`.
* **Mã nguồn Worker:** Đặt tại `n16r8_ver/tools/cloudflare_alert_worker/`. Xác thực Bearer Token, gửi tin nhắn chuẩn CAD/eCall.
* **Lưu NVS:** Lưu cấu hình trong namespace `"motomap"`: `alrt_en`, `alrt_url`, `alrt_key`, `alrt_chats`.

---

## 3. Khắc phục xung đột phần cứng GPIO

> [!WARNING]
> **Xung đột chân GPIO 9 đã được giải quyết:**
> - Chân **GPIO 9** đã được dùng cho **`PIN_ST7789_CS`** (Chip Select màn hình SPI).
> - **Cấu hình chân trong `PinConfig.h`:**
>   - **I2C SDA:** **GPIO 8**
>   - **I2C SCL:** **GPIO 7**
>   - Tuyệt đối không dùng GPIO 26–37 (dành riêng cho Octal Flash & PSRAM).
