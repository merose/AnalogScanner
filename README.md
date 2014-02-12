AnalogScanner
=============

Interrupt-driven analog input scanner class for Arduino.

Purpose
-------
The built-in function `analogRead()` can be used to read any of
the analog pins on the Arduino. However, that method blocks while
waiting for the Analog-to-Digital Converter (ADC) to read the
analog voltage level. By default it takes over 100us to read a
single analog input. If you have a lot of analog sensors that need
to be read frequently, that means you will spend much of the CPU
time busy-waiting for the ADC.

The `AnalogScanner` library avoids busy-waiting by allowing the
caller to set up a scan order for the analog pins. The first ADC
read is requested, but control immediately returns to the caller.
Instead, the ADC read completion interrupt is used to process
the analog value and also start the next ADC read. The pins
specified in the scan order are read in turn, repeatedly, without
requiring the main code to block or busy-wait.

License
-------
Copyright 2014 Mark Rose

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Installation
------------
Click the "Download ZIP" button to download a .zip of the entire
AnalogScanner library. Unpack the .zip and rename the top-level
directory to AnalogScanner. Move or copy that directory to the
`libraries/` directory inside the sketch folder for your Arduino IDE.
You may need to create that `libraries/` directory if this is the
first library you install. Restart the Arduino IDE to make it
available in the Sketch > Import Library... menu.

Basic Usage
-----------
Use the Sketch > Import Library... menu item to add the AnalogScanner
library to your sketch. This will add a line at the top of your
sketch that includes the library definitions:

    #include <AnalogScanner.h>

Or, add that `#include` by hand, instead.

You need to create an instance of the `AnalogScanner` class, usually a
global variable. Set the analog pin scan order and begin scanning.
Then, read the latest analog values whenever you like.

    AnalogScanner scanner;

    void setup() {
        int scanOrder[] = {A0, A1};
        scanner.setScanOrder(2, scanOrder);
        scanner.beginScanning();
        delay(1); // Wait for the first scans to occur.
    }

    void loop() {
        int v0 = scanner.getValue(A0);
        int v1 = scanner.getValue(A1);
        ... process the new values ...
        delay(100); // Wait for new values.
    }

Calls expecting a pin number also accept a zero-relative pin index, where 0=A0, 1=A1, and so on. You can either pass the pin number constant defined by the Arduino libraries (A3, for example), or the corresponding analog pin index (3, for example).

**Note:** If you use a pin index that is not supported, zero will be used instead.

Examples
--------
The `examples/` subdirectory contains `analog_read_all.ino`, an example program that reads each analog input available on the Uno repeatedly (A0 through A6). The values are written to `Serial` once per second.

**Note:** The analog inputs are scanned as fast as possible, usually about 10 pins per millisecond. Only the most recent value is displayed each second by the example program.

Using Read Callbacks
--------------------
If you want to process new values as soon as they are read, you can
use `setCallback()` to specify a function that should be called as
soon as new values are read.

    void processValue(int index, int pin, int value) {
        ... process the new value ...
    }

    ...

    scanner.setCallback(A0, processValue);

The index value is in the range 0 through 15, and represents the zero-relative pin index, where 0=A0 though 15=A15. (Different processors support different numbers of analog pins.) The pin value is the Arduino pin definition constant, from A0 through A15.

**WARNING:** The call to the callback function is processed from within the ADC interrupt handler. This can happen as often as 10,000 times per second, if only one analog pin is in the scan order. The processing function must operate quickly and return. Further, it should not invoke other libraries that are relatively slow, such as `Serial`. Do not write debugging output from the callback handler or you will likely hang the Arduino.

Instead, only use callbacks if you need to perform some processing for each input value, such as applying a digital filter to smooth values from a noisy sensor or to square up edges of an analog signal.

References
----------
* AtMega328 reference manual - http://www.atmel.com/Images/Atmel-8271-8-bit-AVR-Microcontroller-ATmega48A-48PA-88A-88PA-168A-168PA-328-328P_datasheet.pdf

API Reference
-------------
Only the public API is described here. More information about the private class members and the implementation can be found within the source files.

### `AnalogScanner()`

The constructor to create a new instance of the analog input scanner. Most sketches will need only a single instance of the scanner, but one could create multiple instances for different program modes. However, only a single scanner instance can be actively scanning at one time.

The new instance will have an empty scan order, so no reads will be performed, by  default. Set the scanning order by calling `setScanOrder()`, and start scanning by calling `beginScanning()`.

#### `void setCallback(int pin, void (*pFn)(int index, int pin, int value))`

Sets a callback function for an analog pin. The callback function will be invoked whenever a new value is read from the specified pin. Define the callback function like this:

    void myCallback(int index, int pin, int newValue) {
        ... process the value ...
    }

Then set the callback to apply to a pin like this (for pin A2, for example):

    scanner.setCallback(A2, myCallback);

### `void setScanOrder(int n, int order[])`
  
Sets the order in which the analog pins are read. The scan order is specified in an array of integer pin numbers or pin indices (0-relative).

A pin may appear more than once in the scan order to read the pin more frequently. For example, if you want to read pins A0, A1, and A2, but want to read A0 twice as often as the other two, you could set up the scan order as follows:

    int order[] = {A0, A1, A0, A2};
    scanner.setScanOrder(4, order);


### `int getValue(int pin)`
Gets the most recently read value for an analog pin number or pin index.
  
### `void setAnalogReference(int reference)`
Sets the analog voltage reference. See the built-in
Arduino function analogReference().
  
### `void beginScanning()`
Begin scanning the analog pins. Invokes an ADC read from
the first pin in the scan order. Subsequent reads will
be invoked by the ADC interrupt handling code.

### `void endScanning()`
Ends scanning the analog pins. Disables the ADC to save
power.
