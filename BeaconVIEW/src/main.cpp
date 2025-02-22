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
#define MAX_REPEATER_COUNT 4 // Максимальное количество повторителей

BLEScan* pBLEScan;
bool doScan = true;

struct RepeaterConnection {
    BLEClient* client;
    BLERemoteCharacteristic* characteristic;
};

RepeaterConnection repeaters[MAX_REPEATER_COUNT]; // Храним до 4 подключенных повторителей
int connectedRepeaters = 0; // Счетчик подключенных устройств

class MyClientCallback : public BLEClientCallbacks {
    void onConnect(BLEClient* pclient) {
        Serial.println("Connected to BLE server");
        Serial.print("Negotiated MTU: ");
        Serial.println(pclient->getMTU());
    }
    void onDisconnect(BLEClient* pclient) {
        Serial.println("Disconnected from BLE server");
        doScan = true;
    }
};

void notifyCallback(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    Serial.print("Data length: ");
    Serial.println(length);

    Serial.print("Received data (hex): ");
    for (size_t i = 0; i < length; i++) {
        Serial.printf("%02X ", pData[i]);
    }
    Serial.println();

    Serial.print("Received data (text): ");
    for (size_t i = 0; i < length; i++) {
        Serial.print((char)pData[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    BLEDevice::init(DEVICE_NAME);
    BLEDevice::setMTU(23);
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);

    Serial.println("Setup complete, starting scan...");
}

void loop() {
    if (doScan) {
        Serial.println("Scanning for repeaters...");
        BLEScanResults foundDevices = pBLEScan->start(5, false);
        
        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            
            if (device.haveServiceUUID() || device.getServiceUUID().toString() == SERVICE_UUID) {
                Serial.print("Found repeater: ");
                Serial.println(device.getAddress().toString().c_str());

                BLEClient* pClient = BLEDevice::createClient();
                pClient->setClientCallbacks(new MyClientCallback());

                if (pClient->connect(&device)) {    
                    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                    if (pRemoteService) {
                        BLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
                        if (pRemoteCharacteristic) {
                            Serial.println("Subscribing to notifications...");
                            pRemoteCharacteristic->registerForNotify(notifyCallback);
                        }
                    } else {
                        Serial.println("Failed to find characteristic, disconnecting...");
                        pClient->disconnect();
                    }
                } else {
                    Serial.println("Failed to connect to device.");
                }
            }
        }
        
        Serial.printf("Found devices: %d\n", foundDevices.getCount());
        delay(5000);
    }
}
