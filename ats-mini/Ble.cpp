#include "Common.h"
#include "Themes.h"
#include "Remote.h"
#include "Ble.h"

//
// BLE Stub implementation to save IRAM
//

int8_t getBleStatus()
{
  return 0; // Disabled
}

void bleStop()
{
  // No-op
}

void bleInit(uint8_t bleMode)
{
  // No-op
}

int bleDoCommand(Stream* stream, RemoteState* state, uint8_t bleMode)
{
  return 0;
}

void remoteBLETickTime(Stream* stream, RemoteState* state, uint8_t bleMode)
{
  // No-op
}