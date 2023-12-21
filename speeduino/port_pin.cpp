#include "port_pin.h"

/** Create & register a direct manipulation port */
ioPort pinToOutputPort(uint8_t pin) {
  if (isValidPin(pin)) {
    pinMode(pin, OUTPUT);
    return { portOutputRegister(digitalPinToPort(pin)), digitalPinToBitMask(pin) };
  }
  return nullIoPort();
}

/** Create & register a direct manipulation port */
ioPort pinToInputPort(uint8_t pin, uint8_t inputMode /* INPUT or INPUT_PULLUP*/) {
  if (isValidPin(pin)) {
    pinMode(pin, inputMode);
    return { portInputRegister(digitalPinToPort(pin)), digitalPinToBitMask(pin) };
  }
  return nullIoPort();
}


uint8_t pinTranslate(uint8_t rawPin)
{
  if (!isValidPin(rawPin)) {
    return NOT_A_PIN;
  }
  if(rawPin > UINT8_C(BOARD_MAX_DIGITAL_PINS)) { 
    return A8 + (rawPin - UINT8_C(BOARD_MAX_DIGITAL_PINS) - 1U); 
  }
  // It's a valid digital pin.
  return rawPin;
}


uint8_t pinTranslateAnalog(uint8_t rawPin)
{
  if (rawPin >= UINT8_C(BOARD_MAX_IO_PINS)) {
    return NOT_A_PIN;
  }
  switch(rawPin)
  {
    case 0: return A0; break;
    case 1: return A1; break;
    case 2: return A2; break;
    case 3: return A3; break;
    case 4: return A4; break;
    case 5: return A5; break;
    case 6: return A6; break;
    case 7: return A7; break;
    case 8: return A8; break;
    case 9: return A9; break;
    case 10: return A10; break;
    case 11: return A11; break;
    case 12: return A12; break;
    case 13: return A13; break;
  #if BOARD_MAX_ADC_PINS >= 14
      case 14: return A14; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 15
      case 15: return A15; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 16
      case 16: return A16; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 17
      case 17: return A17; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 18
      case 18: return A18; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 19
      case 19: return A19; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 20
      case 20: return A20; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 21
      case 21: return A21; break;
    #endif
    #if BOARD_MAX_ADC_PINS >= 22
      case 22: return A22; break;
    #endif
    default:
      return rawPin;
  }
}