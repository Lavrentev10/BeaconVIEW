#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLEUUID.h>
#include <BLE2902.h>

#define DEVICE_NAME "ESP32_BLE_Receiver"
#define SERVICE_UUID "0000180D-0000-1000-8000-00805F9B34FB"  // UUID сервиса
#define CHARACTERISTIC_UUID "2A37"  // UUID характеристики

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("Connected to BLE server");
    }
    void onDisconnect(BLEClient* pclient) {
        Serial.println("Disconnected from BLE server");
    }
};

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Received data: ");
    Serial.write(pData, length);
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init(DEVICE_NAME);
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    
    while (true) {
        Serial.println("Scanning for repeater...");
        BLEScanResults foundDevices = pBLEScan->start(5, false);
        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            if (device.haveServiceUUID() || device.getServiceUUID().toString().c_str() == "0000180D-0000-1000-8000-00805F9B34FB") {
                // Serial.printf("%d) ServiceUUID = %s\n",i, device.getServiceUUID().toString());
                Serial.printf("Raw UUID: %s\n", device.getServiceUUID().toString().c_str());

                Serial.println("Found repeater, connecting...");
                BLEClient* pClient = BLEDevice::createClient();
                pClient->setClientCallbacks(new MyClientCallback());
                pClient->connect(&device);
                BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                if (pRemoteService) {
                    BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
                    if (pRemoteCharacteristic) {
                        Serial.println("Subscribing to notifications...");
                        pRemoteCharacteristic->registerForNotify(notifyCallback);
                        return;
                    }
                }
                Serial.println("Failed to find characteristic, disconnecting...");
                pClient->disconnect();
            }
        }
        Serial.printf("Found devices: %d\n", foundDevices.getCount());
        delay(5000);
    }
}

void loop() {
    delay(1000); // Основной цикл не выполняет ничего, так как обработка идёт через уведомления
}
