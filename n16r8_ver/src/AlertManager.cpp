#include "AlertManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace {
constexpr size_t kQueueLength = 4;
constexpr uint32_t kWorkerStackSize = 8192;
constexpr UBaseType_t kWorkerPriority = 1;
constexpr BaseType_t kWorkerCore = 0; // Run on Core 0 to prevent UI and GPS contention on Core 1
constexpr uint32_t kHttpTimeoutMs = 7000;
}

AlertManager::AlertManager(SettingsStore* settings)
    : _settings(settings) {}

AlertManager::~AlertManager() {
    if (_workerTaskHandle != nullptr) {
        vTaskDelete(_workerTaskHandle);
        _workerTaskHandle = nullptr;
    }
    if (_alertQueue != nullptr) {
        vQueueDelete(_alertQueue);
        _alertQueue = nullptr;
    }
}

bool AlertManager::begin() {
    if (_alertQueue == nullptr) {
        _alertQueue = xQueueCreate(kQueueLength, sizeof(AlertEvent));
        if (_alertQueue == nullptr) {
            Serial.println("[ALERT] ERROR: Failed to create alert queue!");
            return false;
        }
    }

    if (_workerTaskHandle == nullptr) {
        const BaseType_t result = xTaskCreatePinnedToCore(
            workerTask,
            "AlertWorker",
            kWorkerStackSize,
            this,
            kWorkerPriority,
            &_workerTaskHandle,
            kWorkerCore
        );

        if (result != pdPASS) {
            Serial.println("[ALERT] ERROR: Failed to spawn AlertWorker task!");
            return false;
        }
    }

    Serial.println("[ALERT] Module initialized successfully.");
    return true;
}

bool AlertManager::triggerAlert(const AlertEvent& event) {
    if (_alertQueue == nullptr) {
        return false;
    }

    const BaseType_t sent = xQueueSend(_alertQueue, &event, 0);
    if (sent != pdPASS) {
        Serial.println("[ALERT] WARNING: Alert queue is full, dropping event!");
        return false;
    }

    Serial.println("[ALERT] Event queued successfully.");
    return true;
}

bool AlertManager::triggerTestAlert(const GpsSnapshot& gps) {
    AlertEvent event{};
    strncpy(event.event, "System Alert Test (Manual Trigger)", sizeof(event.event) - 1);
    event.latitude = gps.valid ? gps.latitude : 10.776889;
    event.longitude = gps.valid ? gps.longitude : 106.656000;
    event.speedKmh = gps.valid ? gps.speedKmh : 0.0F;
    event.courseDeg = gps.valid ? gps.courseDeg : 0.0F;
    strncpy(event.status, "Manually triggered via Web Portal", sizeof(event.status) - 1);
    return triggerAlert(event);
}

bool AlertManager::isBusy() const {
    return _isBusy;
}

String AlertManager::lastStatus() const {
    return _lastStatus;
}

void AlertManager::workerTask(void* param) {
    auto* manager = static_cast<AlertManager*>(param);
    manager->runWorker();
}

void AlertManager::runWorker() {
    AlertEvent event;
    while (true) {
        if (xQueueReceive(_alertQueue, &event, portMAX_DELAY) == pdPASS) {
            _isBusy = true;
            Serial.printf("[ALERT] Worker received event: %s\n", event.event);

            if (_settings == nullptr) {
                _lastStatus = "Error: SettingsStore not initialized";
                _isBusy = false;
                continue;
            }

            AlertConfig config;
            if (!_settings->loadAlertConfig(config) || !config.enabled) {
                _lastStatus = "Alerts are currently disabled";
                Serial.println("[ALERT] Alert feature is disabled in settings.");
                _isBusy = false;
                continue;
            }

            if (config.gatewayUrl.isEmpty()) {
                _lastStatus = "Cloudflare Gateway URL not configured";
                Serial.println("[ALERT] Gateway URL is empty. Aborting.");
                _isBusy = false;
                continue;
            }

            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[ALERT] Wi-Fi not connected! Waiting up to 3s...");
                const uint32_t waitStart = millis();
                while (WiFi.status() != WL_CONNECTED && millis() - waitStart < 3000) {
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
            }

            if (WiFi.status() != WL_CONNECTED) {
                _lastStatus = "Error: No Wi-Fi or Hotspot connection";
                Serial.println("[ALERT] Cannot send alert: No Internet connection.");
                _isBusy = false;
                continue;
            }

            sendAlertPayload(event, config);
            _isBusy = false;
        }
    }
}

bool AlertManager::sendAlertPayload(const AlertEvent& event, const AlertConfig& config) {
    JsonDocument doc;
    doc["event"] = event.event;
    doc["latitude"] = event.latitude;
    doc["longitude"] = event.longitude;
    doc["speedKmh"] = event.speedKmh;
    doc["courseDeg"] = event.courseDeg;
    doc["status"] = event.status;
    doc["deviceId"] = "MotoMap-S3";

    // Parse comma-separated Chat IDs
    JsonArray chatArray = doc["chatIds"].to<JsonArray>();
    String rawChats = config.chatIds;
    rawChats.trim();
    int startIdx = 0;
    while (startIdx < rawChats.length()) {
        int commaIdx = rawChats.indexOf(',', startIdx);
        if (commaIdx == -1) {
            commaIdx = rawChats.length();
        }
        String item = rawChats.substring(startIdx, commaIdx);
        item.trim();
        if (item.length() > 0) {
            chatArray.add(item);
        }
        startIdx = commaIdx + 1;
    }

    String jsonBody;
    serializeJson(doc, jsonBody);

    Serial.printf("[ALERT] Dispatching to Gateway: %s\n", config.gatewayUrl.c_str());

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(kHttpTimeoutMs / 1000U);

    HTTPClient http;
    http.setConnectTimeout(kHttpTimeoutMs);
    http.setTimeout(kHttpTimeoutMs);

    if (!http.begin(client, config.gatewayUrl)) {
        _lastStatus = "Error: Cannot connect to Gateway URL";
        Serial.println("[ALERT] http.begin() failed!");
        return false;
    }

    http.addHeader("Content-Type", "application/json");
    if (!config.authKey.isEmpty()) {
        http.addHeader("Authorization", "Bearer " + config.authKey);
    }

    const int httpCode = http.POST(jsonBody);
    bool ok = false;
    if (httpCode >= 200 && httpCode < 300) {
        _lastStatus = "Sent successfully (HTTP " + String(httpCode) + ")";
        Serial.printf("[ALERT] Sent successfully! Response: %s\n", http.getString().c_str());
        ok = true;
    } else {
        _lastStatus = "Gateway error (HTTP " + String(httpCode) + ")";
        Serial.printf("[ALERT] Gateway responded with error code %d: %s\n", httpCode, http.getString().c_str());
    }

    http.end();
    return ok;
}
