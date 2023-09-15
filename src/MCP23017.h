/*
	Library for the Microchip MCP23017 I2C port expander
	(c)sstaub 2023
	Original written by Limor Fried/Ladyada for Adafruit Industries
	under BSD license
*/

#ifndef MCP23017_H
#define MCP23017_H

#include "Arduino.h"
#include "Wire.h"

static const uint8_t MCP23X17_ADDRESS  = 0x20;
static const uint8_t MCP23X17_INT_ERR  = 0xFF;

// Registers Port A
static const uint8_t MCP23X17_IODIRA   = 0x00;   // IODIR:   I/O DIRECTION REGISTER
static const uint8_t MCP23X17_IPOLA    = 0x02;   // IIPOL:   INPUT POLARITY PORT REGISTER
static const uint8_t MCP23X17_GPINTENA = 0x04;   // GPINTEN: INTERRUPT-ON-CHANGE PINS
static const uint8_t MCP23X17_DEFVALA  = 0x06;   // DEFVAL:  DEFAULT VALUE REGISTER
static const uint8_t MCP23X17_INTCONA  = 0x08;   // INTCON:  INTERRUPT-ON-CHANGE CONTROL REGISTER
static const uint8_t MCP23X17_IOCONA   = 0x0A;   // IOCON:   I/O EXPANDER CONFIGURATION REGISTER
static const uint8_t MCP23X17_GPPUA    = 0x0C;   // GPPU:    GPIO PULL-UP RESISTOR REGISTER
static const uint8_t MCP23X17_INTFA    = 0x0E;   // INTF:    INTERRUPT FLAG REGISTER 
static const uint8_t MCP23X17_INTCAPA  = 0x10;   // INTCAP:  INTERRUPT CAPTURED VALUE FOR PORT REGISTER 
static const uint8_t MCP23X17_GPIOA    = 0x12;   // GPIO:    GENERAL PURPOSE I/O PORT REGISTER
static const uint8_t MCP23X17_OLATA    = 0x14;   // OLAT:    OUTPUT LATCH REGISTER 0

// Registers Port B
static const uint8_t MCP23X17_IODIRB   = 0x01;
static const uint8_t MCP23X17_IPOLB    = 0x03;
static const uint8_t MCP23X17_GPINTENB = 0x05;
static const uint8_t MCP23X17_DEFVALB  = 0x07;
static const uint8_t MCP23X17_INTCONB  = 0x09;
static const uint8_t MCP23X17_IOCONB   = 0x0B;
static const uint8_t MCP23X17_GPPUB    = 0x0D;
static const uint8_t MCP23X17_INTFB    = 0x0F;
static const uint8_t MCP23X17_INTCAPB  = 0x11;
static const uint8_t MCP23X17_GPIOB    = 0x13;
static const uint8_t MCP23X17_OLATB    = 0x15;

/**
 * @brief Class for Microchip MCP23017
 * 
 */
class MCP23017 {
public:

	/**
	 * @brief Construct a new MCP23017 object given its HW selected address
	 * 
	 * @param i2cAddr 
	 */
	MCP23017(uint8_t i2cAddr = 0);

	/**
	 * @brief Initializes the MCP23017
	 * 
	 * @param wire alternatives I2C Ports, Wire and Wire1 ...
	 */
	void begin(TwoWire &wire = Wire);

	/**
	 * @brief Sets the pin mode to either INPUT, INPUT_PULLUP or OUTPUT
	 * 
	 * @param pin
	 * @param mode
	 */
	void pinMode(uint8_t pin, uint8_t mode);

	/**
	 * @brief Write HIGH or LOW to a pin.
	 * 
	 * @param pin
	 * @param value HIGH or LOW
	 */
	void digitalWrite(uint8_t pin, uint8_t value);

	/**
	 * @brief Read a pin.
	 * 
	 * @param  pin
	 * @return value 1 for HIGH and 0 for LOW
	 */
	uint8_t digitalRead(uint8_t pin);

	/**
	 * @brief Configures the interrupt system. Both port A and B are assigned to the same configuration.
	 * 
	 * @param mirroring OR both INTA and INTB pins
	 * @param openDrain set the INT pin to value or open drain
	 * @param polarity  set LOW or HIGH on interrupt
	 */
	void interruptSetup(uint8_t mirroring, uint8_t open, uint8_t polarity);

	/**
	 * @brief Setup a pin for interrupt.
	 * 
	 * @param pin
	 * @param mode CHANGE, FALLING, RISING.
	 */
	void enableInterruptPin(uint8_t pin, uint8_t mode);

	/**
	 * @brief Disable a pin for interrupt.
	 * 
	 * @param pin.
	 */
	void disableInterruptPin(uint8_t pin);
	
	/**
	 * @brief Get the last interrupt pin.
	 * 
	 * @return pin
	 */
	uint8_t getLastInterruptPin();

	/**
	 * @brief Get the value of the last interrupt pin.
	 * 
	 * @return value
	 */
	uint8_t getLastInterruptValue();

	/**
	 * @brief Clear Interrupts
	 * 
	 * @return value
	 */
    void clearInterrupts();
	
private:
	uint8_t i2cAddr;
	TwoWire *wire;
	uint8_t bitForPin(uint8_t pin);
	uint8_t regForPin(uint8_t pin, uint8_t portAaddr, uint8_t portBaddr);
	uint8_t read(uint8_t addr);
	void write(uint8_t addr, uint8_t value);
	void update(uint8_t p, uint8_t pValue, uint8_t portAaddr, uint8_t portBaddr);
	};

#endif
