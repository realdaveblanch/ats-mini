#ifndef BLE_H
#define BLE_H

#include "Remote.h"

// Dummy class to satisfy compilation if referenced, or just remove it.
// Since we removed references in .ino, we can just declare the extern functions.

// Disable NordicUART class for now to save IRAM and fix build errors
/*
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <semaphore>

#define NORDIC_UART_SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define NORDIC_UART_CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define NORDIC_UART_CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class NordicUART : public Stream, public BLEServerCallbacks, public BLECharacteristicCallbacks {
  // ... code removed to save IRAM ...
};
extern NordicUART BLESerial;
*/

// Function prototypes
void bleInit(uint8_t bleMode);
void bleStop();
int8_t getBleStatus();
void remoteBLETickTime(Stream* stream, RemoteState* state, uint8_t bleMode);
int bleDoCommand(Stream* stream, RemoteState* state, uint8_t bleMode);

#endif