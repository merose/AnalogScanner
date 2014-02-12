// analog_read_all - Example of using the AnalogScanner library.
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

#include <AnalogScanner.h>

// Creates an instance of the analog pin scanner.
AnalogScanner scanner;

// The order in which we should scan the analog pins.
int scanOrder[] = {A0, A1, A2, A3, A4, A5};
const int SCAN_COUNT = sizeof(scanOrder) / sizeof(scanOrder[0]);

// Sets the scan order to A0 through A5, in turn, and begins reading
// analog values.
void setup() {
  Serial.begin(9600);
  scanner.setScanOrder(SCAN_COUNT, scanOrder);
  scanner.beginScanning();
}

// Forever, display the most recently read values for all 6
// analog inputs available on the Uno.
void loop() {
  for (int i=0; i < SCAN_COUNT; ++i) {
    if (i > 0) {
      Serial.print(" ");
    }
    Serial.print("v");
    Serial.print(i);
    Serial.print("=");
    Serial.print(scanner.getValue(scanOrder[i]));
  }
  
  Serial.println();
  delay(1000);
}

