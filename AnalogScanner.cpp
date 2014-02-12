// AnalogScanner.cpp - Implementation file for the AnalogScanner
//   library for Arduino.
// 
// Copyright 2014 Mark Rose, markrose@acm.org
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This code is designed to support these Arduino boards and
// Atmel processors:
//
// Processor           Boards
// ---------           ------
// ATmega328           Uno, Mini, Nano
//
// ATmega32u4          Leonardo, Micro, Esplora
//
// ATmega2560          Mega 2560
//
// As of this writing, it has only been tested by the author on the
// Uno (ATmega328), however.
//
// The information sources for the code in this library were both
// the Arduino IDE library source files, particularly wiring_analog.c
// and pins_arduino.h, and the Atmel reference manuals.

#include <Arduino.h>
#include <AnalogScanner.h>

// Define cbi() and sbi() for clearing and setting bits in the
// ADC registers.
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// A pointer to the currently active scanner. The ADC interrupt
// handler calls the currently active scanner to process a new
// analog value when available.
AnalogScanner *AnalogScanner::pCurrentScanner = NULL;

// Gets the index (0-15) corresponding to a pin number. If
// the pin number is already an index in the range 0-15, it is
// returned unchanged. Otherwise it is converted to a pin index.
//
// The mapping is A0->0, A1->1, ..., A15->15.
int AnalogScanner::getPinIndex(int pin) {
  switch (pin) {
  case A0: //FALLTHROUGH
  case 0: return 0;
      
  case A1: //FALLTHROUGH
  case 1: return 1;
      
  case A2: //FALLTHROUGH
  case 2: return 2;
      
  case A3: //FALLTHROUGH
  case 3: return 3;
      
  case A4: //FALLTHROUGH
  case 4: return 4;
      
  case A5: //FALLTHROUGH
  case 5: return 5;
      
  case A6: //FALLTHROUGH
  case 6: return 6;
      
  case A7: //FALLTHROUGH
  case 7: return 7;

#ifdef A8
  case A8: //FALLTHROUGH
  case 8: return 8;
#endif

#ifdef A9
  case A9: //FALLTHROUGH
  case 9: return 9;
#endif

#ifdef A10
  case A10: //FALLTHROUGH
  case 10: return 10;
#endif

#ifdef A11
  case A11: //FALLTHROUGH
  case 11: return 11;
#endif

#ifdef A12
  case A12: //FALLTHROUGH
  case 12: return 12;
#endif

#ifdef A13
  case A13: //FALLTHROUGH
  case 13: return 13;
#endif

#ifdef A14
  case A14: //FALLTHROUGH
  case 14: return 14;
#endif

#ifdef A15
  case A15: //FALLTHROUGH
  case 15: return 15;
#endif

  default:
    return 0;
  }
}

// Gets the Arduino pin number corresponding to an
// analog input index. For example, 0 corresponds to
// A0, 1 to A1, and so on.
int AnalogScanner::getPinForIndex(int index) {
  switch (index) {
  case 0: return A0;
  case 1: return A1;
  case 2: return A2;
  case 3: return A3;
  case 4: return A4;
  case 5: return A5;
  case 6: return A6;
  case 7: return A7;

#ifdef A8
  case 8: return A8;
#endif

#ifdef A9
  case 9: return A9;
#endif

#ifdef A10
  case 10: return A10;
#endif

#ifdef A11
  case 11: return A11;
#endif

#ifdef A12
  case 12: return A12;
#endif

#ifdef A13
  case 13: return A13;
#endif

#ifdef A14
  case 14: return A14;
#endif

#ifdef A15
  case 15: return A15;
#endif

  default:
    return A0;
  }
}

// Creates a new instance of the analog input scanner. Initializes
// all scanned values to zero and all callbacks to null. Clears the
// scan order and sets the analog reference voltage to the default.
AnalogScanner::AnalogScanner() {
  for (int i=0; i < ANALOG_INPUTS; ++i) {
    values[i] = 0;
    pCallback[i] = NULL;
  }
  
  scanOrderSize = 0;
  currentIndex = -1;
  analogRef = DEFAULT;
}

// Sets a callback function for a particular analog pin. The
// function is called by the interrupt handling code as soon
// as a new analog value is available.
void AnalogScanner::setCallback(int pin, void (*p)(int index, int pin, int value)) {
  pCallback[getPinIndex(pin)] = p;
}

// Sets the scan order for the analog pins. The same pin may
// be specified multiple times if a pin should be sampled more
// often.
void AnalogScanner::setScanOrder(int n, int order[]) {
  scanOrderSize = min(SCAN_ORDER_MAX, n);
  for (int i=0; i < scanOrderSize; ++i) {
    scanOrder[i] = getPinIndex(order[i]);
  }
}

// Gets the most recently read value for an analog pin.
int AnalogScanner::getValue(int pin) {
  noInterrupts();
  int value = values[getPinIndex(pin)];
  interrupts();
    
  return value;
}

// Sets the analog reference voltage. See the built-in Arduino
// function analogReference() for details.
void AnalogScanner::setAnalogReference(int reference) {
  analogRef = reference & 3;
}

// Begins scanning the analog input pins.
void AnalogScanner::beginScanning() {
  pCurrentScanner = this;
  sbi(ADCSRA, ADEN); // Enable the ADC.
  delay(1);
  cbi(ADMUX, ADLAR); // Make sure the ADC value it right-jusitified.
  sbi(ADCSRA, ADIE); // Enable ADC complete interrupts.

  startNextScan();    
}

// Ends scanning the analog input pins. Disables the ADC to
// save power.
void AnalogScanner::endScanning() {
  cbi(ADCSRA, ADEN); // Disable the ADC.
  cbi(ADCSRA, ADIE); // Disable ADC complete interrupts.
}

// Starts the next ADC read.
void AnalogScanner::startNextScan() {
  if (scanOrderSize > 0) {
    if (++currentIndex >= scanOrderSize) {
      currentIndex = 0;
    }
    int index = scanOrder[currentIndex];
#ifdef MUX5
    // Set whether we're reading from inputs 0-7 or 8-15.
    ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((index >> 3) & 0x01) << MUX5);
#endif
    ADMUX = (analogRef << 6) | (index & 7);
    sbi(ADCSRA, ADSC); // Start the ADC conversion.
  }
}

// Processes a new value from the ADC.
void AnalogScanner::processScan() {
  // We must read ADCL first, which locks ADCH until it is read.
  int low = ADCL;
  int high = ADCH;
  int index = scanOrder[currentIndex];
  values[index] = (high << 8) | low;

  // Invoke the next scan before the callback, to make the
  // read rate more uniform.
  startNextScan();

  if (pCallback[index] != NULL) {
    pCallback[index](index, getPinForIndex(index), values[index]);
  }
}

// Handles the ADC completion interrupt by requesting that the
// current scanner read teh newly available analog input value.
void AnalogScanner::scanComplete() {
  pCurrentScanner->processScan();
}

// Defines an ADC interrupt processing routine that asks the
// currently active scanner to process a newly scanned value.
ISR(ADC_vect) {
  AnalogScanner::scanComplete();
}
