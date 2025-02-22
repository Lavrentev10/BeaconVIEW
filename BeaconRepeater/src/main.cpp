#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <string>
#include <algorithm>


#define DEVICE_NAME "ESP32_BLE_Repeater"
#define TARGET_DEVICE_NAME "BT5.2 Mouse"
#define SCAN_TIME 5 // Время сканирования в секундах

std::string RepeaterUUID = "2";

BLEServer *pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;
BLEScan *pBLEScan;
std::string lastDeviceInfo = "";

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        if (advertisedDevice.getName() == TARGET_DEVICE_NAME) {

            int rssi = advertisedDevice.getRSSI();
            std::string address = advertisedDevice.getAddress().toString();
            address.erase(std::remove(address.begin(), address.end(), ':'), address.end());
            std::string rssiStr = std::to_string(rssi);

            // Удаляем символ '-' из строки
            rssiStr.erase(std::remove(rssiStr.begin(), rssiStr.end(), '-'), rssiStr.end());

            std::string deviceInfo = RepeaterUUID + ";" + address + 
                                     ";" + rssiStr;
            if (deviceInfo != lastDeviceInfo) {
                lastDeviceInfo = deviceInfo;
                Serial.println(("Found target device: " + deviceInfo).c_str());
                if (pCharacteristic) {
                    std::string message = deviceInfo + '\0'; // Добавляем завершающий символ
                    Serial.print("Effective MTU: ");
                    Serial.println(BLEDevice::getMTU());

                    if (deviceInfo.length() > 20) {
                        std::string part1 = deviceInfo.substr(0, 20);
                        std::string part2 = deviceInfo.substr(20);
                        pCharacteristic->setValue(part1);
                        pCharacteristic->notify();
                        delay(10); // Небольшая задержка между уведомлениями
                        pCharacteristic->setValue(part2);
                        pCharacteristic->notify();
                    } else {
                        pCharacteristic->setValue(deviceInfo);
                        pCharacteristic->notify();
                    }
                }
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    BLEDevice::init(DEVICE_NAME);
    BLEDevice::setMTU(50);
    
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

    delay(1000);
}

void loop() {
    Serial.println("Scanning...");
    pBLEScan->start(SCAN_TIME, false);
    delay(2000);
}
