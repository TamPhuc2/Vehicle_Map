#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "MotoMapTypes.h"
#include "SettingsStore.h"

class AlertManager {
public:
    explicit AlertManager(SettingsStore* settings);
    ~AlertManager();

    bool begin();
    bool triggerAlert(const AlertEvent& event);
    bool triggerTestAlert(const GpsSnapshot& gps);
    bool isBusy() const;
    String lastStatus() const;

private:
    SettingsStore* _settings = nullptr;
    QueueHandle_t _alertQueue = nullptr;
    TaskHandle_t _workerTaskHandle = nullptr;
    volatile bool _isBusy = false;
    String _lastStatus = "Idle";

    static void workerTask(void* param);
    void runWorker();
    bool sendAlertPayload(const AlertEvent& event, const AlertConfig& config);
};
