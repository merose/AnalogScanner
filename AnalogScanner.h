#ifndef AnalogScanner_h
#define AnalogScanner_h

// AnalogScanner.h - Header file for the AnalogScanner library
//   for Arduino.
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

// Implements an object that can read the AVR analog inputs without
// blocking. This class allows the caller to specify a scan order
// for the analog inputs, then begin scanning. At that point the
// scanner object will iterate over each specified analog input
// pin, starting an analog read and processing the analog value
// in an ADC interrupt handler. The interrupt handling code then
// initiates the read of the next analog pin. In this way there
// is no blocking during the analog reads.
//
// See the accompanying documentation for API details.

class AnalogScanner {

 private:
  // A pointer to the currently active scanner object. Only
  // one scanner object can initiate ADC reads at one time.
  static AnalogScanner *pCurrentScanner;

  // The maximum number of analog input pins. Some Atmel
  // chips support fewer inputs than this.
  static const int ANALOG_INPUTS = 15;

  // The maximum length of the analog input scan order.
  static const int SCAN_ORDER_MAX = 50;

  // The analog input values.
  volatile int values[ANALOG_INPUTS];

  // The scan order. the same pin may be specified multiple
  // times, in order to read some pins more often.
  int scanOrder[ANALOG_INPUTS];

  // The size of the scan order.
  int scanOrderSize;

  // The current index within the scan order.
  int currentIndex;

  // An array of pointers to callback routines invoked when
  // new values are available.
  void (*pCallback[ANALOG_INPUTS])(int index, int pin, int value);

  // The analog voltage reference value, as defined in the
  // built-in analogReference() Arduino function.
  int analogRef;
  
  // Gets the pin index for a pin number. For example, A0 has
  // the value 14 on the AtMega328, but has pin index 0 for
  // specifying in the ADMUX register.
  int getPinIndex(int pin);

  // Gets the pin number corresponding to an ADC pin index
  // index. For example, 0 corresponds to pin A0.
  int getPinForIndex(int index);

  // Processes a new ADC input value for the current scan pin.
  void processScan();

  // Starts an ADC read of the next pin in the scan order.
  void startNextScan();

 public:
  // Creates a new instance of the analog input scanner.
  AnalogScanner();
  
  // Sets a callback function for an analog pin. The callback
  // function will be invoked as soon as a new ADC value is
  // available.
  void setCallback(int pin, void (*p)(int index, int pin, int value));
  
  // Sets the scan order. A single pin may be specified
  // multiple times in the scan order to increase the rate
  // at which it is read.
  void setScanOrder(int n, int order[]);
  
  // Gets the most recently read value for an analog pin.
  int getValue(int pin);
  
  // Sets the analog voltage reference. See the built-in
  // Arduino function analogReference().
  void setAnalogReference(int reference);
  
  // Begin scanning the analog pins. Invokes an ADC read from
  // the first pin in the scan order. Subsequent reads will
  // be invoked by the ADC interrupt handling code.
  void beginScanning();

  // Ends scanning the analog pins. Disables the ADC to save
  // power.
  void endScanning();
  
  // Called by the ADC interrupt handler to process a
  // new analog input value.
  static void scanComplete();

};

#endif //AnalogScanner_h
