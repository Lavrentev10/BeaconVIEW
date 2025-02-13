#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define DEVICE_NAME "ESP32_BLE_Repeater"
#define TARGET_DEVICE_NAME "BT5.2 Mouse"
#define SCAN_TIME 5 // Время сканирования в секундах

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
BLEScan *pBLEScan;
std::string lastDeviceInfo = "";

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == TARGET_DEVICE_NAME) {
            std::string deviceInfo = "MAC: " + advertisedDevice.getAddress().toString() + 
                                     " RSSI: " + std::to_string(advertisedDevice.getRSSI());
            if (deviceInfo != lastDeviceInfo) {
                lastDeviceInfo = deviceInfo;
                Serial.println(("Found target device: " + deviceInfo).c_str());
                if (pCharacteristic) {
                    pCharacteristic->setValue(deviceInfo);
                    pCharacteristic->notify();
                }
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init(DEVICE_NAME);
    
    // Настройка сервера BLE
    pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(BLEUUID("180D")); // UUID сервиса
    pCharacteristic = pService->createCharacteristic(
        BLEUUID((uint16_t)0x2A37),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(pService->getUUID());
    pAdvertising->start();
    
    // Настройка BLE-сканирования
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
}

void loop() {
    Serial.println("Scanning...");
    pBLEScan->start(SCAN_TIME, false);
    delay(2000);
}
