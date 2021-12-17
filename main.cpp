/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
#include <BLE2902.h>
#include <analogWrite.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define LED 4
float txValue = 0;
int FADE_PEDIOD = 0;
unsigned long fadeStartTime;
boolean trigger = false;

#define SERVICE_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      Serial.print("Received Value: ");

      for (int i = 0; i < rxValue.length(); i++)
      {
        Serial.print(rxValue[i]);
      }

      Serial.println();

      // Do stuff based on the command received from the app
      if (rxValue.find("A") != -1)
      {
        Serial.println("Turning ON!");
        trigger = true;
      }
      else if (rxValue.find("B") != -1)
      {
        Serial.println("Turning OFF!");
        FADE_PEDIOD = 0;
        analogWrite(LED, 0, 255);
      }

      Serial.println();
      Serial.println("*********");
    };
  }
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("ESP32 BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue("Hello World says Neil");
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop()
{
  if (trigger)
  {
    fadeStartTime = millis();
    trigger = false;
    FADE_PEDIOD = 600000;
  }
  unsigned long progress = millis() - fadeStartTime;

  if (progress <= FADE_PEDIOD)
  {
    long brightness = map(progress, 0, FADE_PEDIOD, 0, 255);
    analogWrite(LED, brightness, 255);
  }
}
