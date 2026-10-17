#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define LED_PIN 2
#define BUTTON_PIN 27

BLEServer *pServer;

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789000"
#define CHARACTERISTIC_UUID "12345678-1234-1234-1234-123456789001"


class CommandCallback : public BLECharacteristicCallbacks {

  void onWrite(BLECharacteristic *pCharacteristic) {

    String value = pCharacteristic->getValue().c_str();

    Serial.print("Received: ");
    Serial.println(value);

    if (value == "RING") {

      Serial.println("LED ON");

      digitalWrite(LED_PIN, HIGH);
    }

    else if (value == "STOP") {

      Serial.println("LED OFF");

      digitalWrite(LED_PIN, LOW);
    }
  }
};


void setup() {

  Serial.begin(115200);

  // GPIO
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(LED_PIN, LOW);


  // BLE
  BLEDevice::init("Quynh");

  pServer = BLEDevice::createServer();


  // Service
  BLEService *pService =
      pServer->createService(SERVICE_UUID);


  // Characteristic
  BLECharacteristic *pCharacteristic =
      pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
      );

  pCharacteristic->setCallbacks(
      new CommandCallback()
  );


  // Start Service
  pService->start();


  // Advertising
  BLEAdvertising *pAdvertising =
      BLEDevice::getAdvertising();

  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);

  BLEDevice::startAdvertising();


  Serial.println("--------------------------");
  Serial.println("MiniSmartTag started");
  Serial.println("Waiting for BLE command...");
  Serial.println("--------------------------");
}


void loop() {

}