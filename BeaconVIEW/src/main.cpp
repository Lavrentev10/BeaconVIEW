#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 1; // Время сканирования в секундах
const std::string targetDeviceName = "BT5.2 Mouse"; // Фильтр по имени

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        std::string deviceName = advertisedDevice.getName();
        
        if (deviceName == targetDeviceName) { // Фильтр по имени устройства
            Serial.printf("Найдено устройство: %s | RSSI: %d dBm\n", 
                          advertisedDevice.toString().c_str(), 
                          advertisedDevice.getRSSI());
        }
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("Запуск BLE-сканирования...");

    BLEDevice::init("ESP32_BLE_Scanner"); // Инициализация BLE
    BLEScan* pBLEScan = BLEDevice::getScan(); // Получаем сканер
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); // Активное сканирование
    pBLEScan->start(scanTime, false);
}

void loop() {
    // Запуск сканирования каждые 5 секунд
    BLEScan* pBLEScan = BLEDevice::getScan();
    BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
    // Serial.printf("Завершено сканирование. Найдено устройств: %d\n", foundDevices.getCount());
    delay(1000);
}
