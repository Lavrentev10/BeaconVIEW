#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "TIMA";         // Ваш SSID
const char* password = "sasasasa"; // Ваш пароль от Wi-Fi

void setup() {
  // Инициализация серийного порта
  Serial.begin(115200);
  delay(1000);

  // Подключение к Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Подключение к Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Подключено к Wi-Fi: ");
  Serial.println(WiFi.localIP());

  // Инициализация OTA
  ArduinoOTA.setHostname("esp32-ota"); // Установите имя устройства
  ArduinoOTA.begin();
}

void loop() {
  // Обработка OTA запросов
  ArduinoOTA.handle();

  // Ваш основной код
}
