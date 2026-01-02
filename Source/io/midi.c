#include "midi.h"
#include "../mGB.h"
#include "midi_asm.h"
#include "serial.h"

uint8_t statusByte;
uint8_t addressByte;
uint8_t valueByte;
uint8_t capturedAddress;

static volatile uint8_t *MIDI_IN = (volatile uint8_t *)0xB000;
static volatile uint8_t *MIDI_PACKET = (volatile uint8_t *)0xB001;

void updateMidiBuffer(void)
{
#if 0
  if (serialBufferReadPosition == serialBufferPosition) {
    return;
  }

  serialBufferReadPosition++; // unsigned overflow from 255 -> 0 is automatic.

  uint8_t byte = serialBuffer[serialBufferReadPosition];

  // STATUS BYTE
  if (byte & MIDI_STATUS_BIT) {
    if ((byte & MIDI_STATUS_SYSTEM) == MIDI_STATUS_SYSTEM) {
      return;
    }
    statusByte = byte;
    capturedAddress = false;
    systemIdle = false;
    return;
  }

  // 2ND BYTE (note/CC control)
  if (!capturedAddress) {
    capturedAddress = true;
    addressByte = byte;
    systemIdle = false;
    return;
  }

  // 3RD BYTE (velocity/value)
  capturedAddress = false;
  valueByte = byte;
  systemIdle = false;

  switch ((statusByte >> 4) & 0x0F) {
  case MIDI_STATUS_PB:
    asmEventMidiPB();
    break;
  case MIDI_STATUS_CC:
    asmEventMidiCC();
    break;
  case MIDI_STATUS_NOTE_ON:
    asmEventMidiNote();
    break;
  case MIDI_STATUS_NOTE_OFF:
    asmEventMidiNoteOff();
    break;
  case MIDI_STATUS_PC:
    asmEventMidiPC();
    break;
  }
#else

  ENABLE_RAM_MBC1;
  SWITCH_RAM_MBC1(0);
  SWITCH_16_8_MODE_MBC1;

  // packets are sent in usb midi packet fmt
  // 0: packet len
  // 1: CIN/CABLE
  // 2: status
  // 3: d0
  // 4: d1

  if (MIDI_IN[0] != 0)
  {
    // data received, block ui
    systemIdle = false;

    statusByte = MIDI_IN[2];
    addressByte = MIDI_IN[3];
    valueByte = MIDI_IN[4];
    
    // stop recv
    MIDI_IN[0] = 0;

    switch ((statusByte >> 4) & 0x0F)
    {
    case MIDI_STATUS_PB:
      asmEventMidiPB();
      break;
    case MIDI_STATUS_CC:
      asmEventMidiCC();
      break;
    case MIDI_STATUS_NOTE_ON:
      asmEventMidiNote();
      break;
    case MIDI_STATUS_NOTE_OFF:
      asmEventMidiNoteOff();
      break;
    case MIDI_STATUS_PC:
      asmEventMidiPC();
      break;
    }
    serialBufferPosition++;
  }

  DISABLE_RAM_MBC1;

#endif
}
