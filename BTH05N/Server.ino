#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// LED RGB trên board ESP32-S3
#define RGB_PIN     48  // Đổi thành 38 nếu LED board dùng GPIO38
#define BRIGHTNESS  40

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789000"
#define CHARACTERISTIC_UUID "12345678-1234-1234-1234-123456789001"

const char *DEVICE_NAME = "Toan";

BLEServer *pServer;

void setColor(uint8_t r, uint8_t g, uint8_t b) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  rgbLedWrite(RGB_PIN, r, g, b);
#else
  neopixelWrite(RGB_PIN, r, g, b);
#endif
}

class ServerCallback : public BLEServerCallbacks {
  void onConnect(BLEServer *s) override {
    Serial.println("Dien thoai da ket noi");
  }

  void onDisconnect(BLEServer *s) override {
    Serial.println("Da ngat ket noi -> phat BLE lai");
    BLEDevice::startAdvertising();
  }
};

class CommandCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) override {
    String value = pCharacteristic->getValue().c_str();

    value.trim();
    value.toUpperCase();

    Serial.print("Nhan lenh: ");
    Serial.println(value);

    if (value == "RED") {
      setColor(BRIGHTNESS, 0, 0);
    } else if (value == "GREEN") {
      setColor(0, BRIGHTNESS, 0);
    } else if (value == "BLUE") {
      setColor(0, 0, BRIGHTNESS);
    } else if (value == "WHITE") {
      setColor(BRIGHTNESS, BRIGHTNESS, BRIGHTNESS);
    } else if (value == "OFF") {
      setColor(0, 0, 0);
    } else {
      Serial.println("Lenh khong hop le");
      Serial.println("Dung: RED, GREEN, BLUE, WHITE, OFF");
      return;
    }

    Serial.print("LED -> ");
    Serial.println(value);
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  setColor(0, 0, 0);

  // Khoi tao BLE voi ten tvq
  BLEDevice::init(DEVICE_NAME);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic =
      pService->createCharacteristic(
          CHARACTERISTIC_UUID,
          BLECharacteristic::PROPERTY_WRITE
      );

  pCharacteristic->setCallbacks(new CommandCallback());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  // Ten thiet bi trong goi quang ba chinh
  BLEAdvertisementData advData;
  advData.setFlags(0x06);
  advData.setName(DEVICE_NAME);
  pAdvertising->setAdvertisementData(advData);

  // UUID dich vu trong goi scan response
  BLEAdvertisementData scanData;
  scanData.setCompleteServices(BLEUUID(SERVICE_UUID));
  pAdvertising->setScanResponseData(scanData);

  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();

  // Nhay xanh la bao khoi dong
  setColor(0, BRIGHTNESS, 0);
  delay(300);
  setColor(0, 0, 0);

  Serial.println("--------------------------");
  Serial.println("Toan da khoi dong");
  Serial.println("Lenh: RED, GREEN, BLUE, WHITE, OFF");
  Serial.println("--------------------------");
}

void loop() {
}