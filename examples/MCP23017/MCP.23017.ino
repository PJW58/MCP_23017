// Attach two LED's to pin 0 and 1 (bank GPA0 and GPA1)
// and two buttons to pin 8 and 9 (bank GPB0 and GPB1).

#include "MCP23017.h"

MCP23017 ioExpander;

void setup() {
	ioExpander.begin();
	ioExpander.pinMode(8, INPUT_PULLUP);
	ioExpander.pinMode(9, INPUT_PULLUP);
	ioExpander.pinMode(0, OUTPUT);
	ioExpander.pinMode(1, OUTPUT);
	ioExpander.digitalWrite(0, LOW);
	ioExpander.digitalWrite(1, LOW);
	}

void loop() {
	ioExpander.digitalWrite(0, !ioExpander.digitalRead(8));
	ioExpander.digitalWrite(1, !ioExpander.digitalRead(9));
	}