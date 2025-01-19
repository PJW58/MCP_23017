#include "MCP23017.h"

MCP23017::MCP23017(uint8_t i2cAddr) {
	if (i2cAddr > 7) i2cAddr = 7;
	this->i2cAddr = i2cAddr;
	}

void MCP23017::begin(TwoWire &wire) {
	this->wire = &wire;
	this->wire->begin();
	write(MCP23X17_IODIRA, 0xff);
	write(MCP23X17_IODIRB, 0xff);
	}

void MCP23017::pinMode(uint8_t pin, uint8_t mode) {
	update(pin, !mode, MCP23X17_IODIRA, MCP23X17_IODIRB);
	update(pin, 0, MCP23X17_GPPUA, MCP23X17_GPPUB);
	if ( mode == INPUT_PULLUP ) {
		update(pin, 1, MCP23X17_IODIRA, MCP23X17_IODIRB);
		update(pin, 1, MCP23X17_GPPUA, MCP23X17_GPPUB);
		}
	}

void MCP23017::digitalWrite(uint8_t pin, uint8_t value) {
	uint8_t gpio;
	uint8_t bit = bitForPin(pin);
	uint8_t regAddr = regForPin(pin, MCP23X17_OLATA, MCP23X17_OLATB);
	gpio = read(regAddr);
	bitWrite(gpio, bit, value);
	regAddr = regForPin(pin, MCP23X17_GPIOA, MCP23X17_GPIOB);
	write(regAddr, gpio);
	}

uint8_t MCP23017::digitalRead(uint8_t pin) {
	uint8_t bit = bitForPin(pin);
	uint8_t regAddr = regForPin(pin, MCP23X17_GPIOA, MCP23X17_GPIOB);
	return (read(regAddr) >> bit) & 0x1;
	}


void MCP23017::interruptSetup(uint8_t mirroring, uint8_t openDrain, uint8_t polarity) {
 	// Note: Unlike all other registers which are not shared between 
    // the two ports (Port A and Port B), there is one register (IOCON).
    // which is shared between the ports and affects both equally.
	uint8_t ioconfValue = read(MCP23X17_IOCONA);
	bitWrite(ioconfValue, IOCON_MIRROR, mirroring);
	bitWrite(ioconfValue, IOCON_ODR,    openDrain);
	bitWrite(ioconfValue, IOCON_INTPOL, polarity);
	write(MCP23X17_IOCONA, ioconfValue);	
	}

void MCP23017::disableInterrupts() {
	//Turns off the enable bits for all 16 ports
	write(MCP23X17_GPINTENA, 0);
	write(MCP23X17_GPINTENB, 0);
	};
	
void MCP23017::enableInterruptPin(uint8_t pin, uint8_t mode) {
	// set the pin interrupt control (0 means change, 1 means compare against given value);
	// if the mode is not CHANGE, we need to set up a default value, different value triggers interrupt
	update(pin, (mode != CHANGE), MCP23X17_INTCONA, MCP23X17_INTCONB);
	// In a RISING interrupt the default value is 0, interrupt is triggered when the pin goes to 1.
	// In a FALLING interrupt the default value is 1, interrupt is triggered when pin goes to 0.
	update(pin, (mode == FALLING), MCP23X17_DEFVALA, MCP23X17_DEFVALB);
	// enable the pin for interrupts
	update(pin, 1, MCP23X17_GPINTENA, MCP23X17_GPINTENB);
	}
	
void MCP23017::disableInterruptPin(uint8_t pin) {
	// disable the pin for interrupts
	update(pin, 0, MCP23X17_GPINTENA, MCP23X17_GPINTENB);
	}
	
uint8_t MCP23017::getLastInterruptPin() {
	uint8_t intf;
	intf = read(MCP23X17_INTFA);
	for(uint8_t i = 0; i < 8; i++) if (bitRead(intf, i)) return i;
	intf = read(MCP23X17_INTFB);
	for(uint8_t i = 0; i < 8; i++) if (bitRead(intf, i)) return i+8;
	return MCP23X17_INT_ERR;
	}

uint8_t MCP23017::getLastInterruptValue( uint8_t intpin = 255 ) {
	// 01/19/25 - PJW:
	// uint8_t intPin = getLastInterruptPin();
	if !( intpin == 255 ) intPin = getLastInterruptPin();
	if(intPin != MCP23X17_INT_ERR) {
		uint8_t intcapreg = regForPin(intPin, MCP23X17_INTCAPA, MCP23X17_INTCAPB);
		uint8_t bit = bitForPin(intPin);
		return (read(intcapreg) >> bit) & (0x01);
		}
	return MCP23X17_INT_ERR;
	}

void MCP23017::clearInterrupts() {
	read( MCP23X17_INTCAPA );
	read( MCP23X17_INTCAPB );	
    }
	
uint8_t MCP23017::bitForPin(uint8_t pin) {
    return pin % 8;
    }

uint8_t MCP23017::regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr) {
    return(pin < 8) ? portAaddr : portBaddr;
    }

uint8_t MCP23017::read(uint8_t addr) {
	wire->beginTransmission(MCP23X17_ADDRESS | i2cAddr);
	wire->write(addr);
	wire->endTransmission();
	wire->requestFrom(MCP23X17_ADDRESS | i2cAddr, 1);
	return wire->read();
	}

void MCP23017::write(uint8_t regAddr, uint8_t regValue) {
	wire->beginTransmission(MCP23X17_ADDRESS | i2cAddr);
	wire->write(regAddr);
	wire->write(regValue);
	wire->endTransmission();
	}

void MCP23017::update(uint8_t pin, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr) {
	uint8_t regValue;
	uint8_t regAddr = regForPin(pin, portAaddr, portBaddr);
	uint8_t bit = bitForPin(pin);
	regValue = read(regAddr);
	bitWrite(regValue, bit, pValue);
	write(regAddr, regValue);
	}
