# MotoMap Zalo Emergency Alert Gateway (Cloudflare Worker)

Gateway trung gian bảo mật chuyển tiếp các bản tin cảnh báo khẩn cấp từ thiết bị MotoMap (ESP32-S3) tới Zalo Bot Platform.

## 1. Tính năng nổi bật
- **Zero Hardcoding**: Không lưu bất kỳ Token hay Chat ID nào trong code.
- **Tiêu chuẩn eCall/CAD**: Bản tin chuẩn hóa, chuyên nghiệp, không dùng biểu tượng cảm xúc thừa thãi.
- **Hỗ trợ đa người nhận**: Gửi song song đến danh sách Chat ID do ESP32 truyền lên.
- **Xác thực bảo mật**: Xác thực Bearer Token giữa ESP32 và Cloudflare.

## 2. Hướng dẫn thiết lập & Triển khai

### Bước 1: Cài đặt Wrangler CLI
```bash
npm install
```

### Bước 2: Đăng nhập vào Cloudflare
```bash
npx wrangler login
```

### Bước 3: Cấu hình các Secret mã hóa (Bắt buộc)
Lưu ý: Các lệnh này sẽ mã hóa an toàn trên cloud của Cloudflare, không lưu vào code.

```bash
# 1. Nhập Bot Token nhận được từ "Zalo Bot Manager"
npx wrangler secret put ZALO_BOT_TOKEN

# 2. Khóa bí mật giữa ESP32 và Gateway (ví dụ: MotomapSecretKey2026)
npx wrangler secret put GATEWAY_AUTH_KEY

# 3. (Tùy chọn) Chat ID mặc định dự phòng nếu ESP32 không gửi kèm chatId
npx wrangler secret put ZALO_DEFAULT_CHAT_ID
```

### Bước 4: Triển khai (Deploy)
```bash
npx wrangler deploy
```
Sau khi deploy thành công, bạn sẽ nhận được một đường link URL có dạng:
`https://motomap-zalo-gateway.<subdomain>.workers.dev`

Nhập URL này và `GATEWAY_AUTH_KEY` vào trang cài đặt của MotoMap trên ESP32.
