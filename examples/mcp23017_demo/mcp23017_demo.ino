/*
=============================================================================
  MCP23017 Demo
-----------------------------------------------------------------------------
  Copyright 2023, Paul J. West
-----------------------------------------------------------------------------
  Libraries used:
    Wire:      Default I2C Wire library
    MCP_23017: Version 23.9.0 
-----------------------------------------------------------------------------
  This program demonstrates the use of the MCP23017 16 Port I/O expander.

  All 16 ports are used, twelve to control 4 RGB LED's, and the final
  four for input from four buttons to control said LED'S. 
-----------------------------------------------------------------------------
  The program also demonstrates controlling RGB LED's with digital ports
  instead of using PWM on analog ports. This limits us to seven colors.
-----------------------------------------------------------------------------
  Interrupts are used to signal the button inputs rather than polling. 
  The MCP23017 Port Expander is capable of generating two interrupts.

  INTa - For changes on Ports PA0..PA7 [ 0..7 ]
  INTb - For changes on Ports PB0..PB7 [ 8..15 ]

  The controller also allows the interrupts to be merged into one.
  Since we don't realy need two, we'll go ahead and merge them and
  only use INTa.
=============================================================================
*/
#include <Wire.h>
#include <MCP23017.h>  

// Constants to define MCP23XXX
const uint8_t MCP23XXX_ADDRESS   = 7;       // Address on I2C bus | 0x20
const bool    MCP23XXX_MIRROR    = true;    // Mirror the two interrupts
const bool    MCP23XXX_OPENDRAIN = true;    // Use Open Drain for the interrupts

// Create an instance of the MCP23XXX class
MCP23017 mcp( MCP23XXX_ADDRESS );  

// Pin Numbers on Expander [0 .. 15]
const uint8_t LED_RED = 0;
const uint8_t LED_GRN = 1;
const uint8_t LED_BLU = 2;
const uint8_t BUTTON_A = 12;
const uint8_t BUTTON_B = 13;
const uint8_t BUTTON_C = 14;
const uint8_t BUTTON_D = 15;

// Pin Numbers used on the ESP32
const uint8_t I2C_SDA = SDA;
const uint8_t I2C_SCL = SCL;
const uint8_t INTA = (33); 

// The original 8 color ANSI standard
const uint8_t BLK = 0b0000;          //  0=off
const uint8_t RED = 0b0001;          //  1=red
const uint8_t GRN = 0b0010;          //  2=green
const uint8_t YEL = RED & GRN;       //  3=yellow
const uint8_t BLU = 0b0100;          //  4=blue
const uint8_t MAG = RED & BLU;       //  5=magenta
const uint8_t CYN = GRN & BLU;       //  6=cyan
const uint8_t WHT = RED & GRN & BLU; //  7=white

// A few other useful constants
const unsigned long SERIAL_SPEED = 115200; 
const uint32_t  DEBOUNCE_TIME    = 125;
const uint32_t  QUARTER_SECOND   = 250;
const uint32_t  HALF_SECOND      = 500;
const uint32_t  SHORT_DELAY      = 10;

//=============================================================================
//  isrButton -  Structures and ISR to handle notification of button presses 
//-----------------------------------------------------------------------------
//  Since this is an ISR, we can't do any actual I/O with the I/O expander.
//  We will set a flag that a button has been pressed so the main loop
//  can check it on the next pass.
//=============================================================================
struct Button {
    bool pressed;
    uint32_t lastPress;
};
 
Button button = { false, 0 };

void IRAM_ATTR isrButton( void* arg ) {
    Button* btn = static_cast<Button*>(arg);

    // Once every 50 days millis() overflows back to zero 
    uint32_t ms = millis();
    if (ms < btn->lastPress) btn->lastPress = 0;

    if ( ( ms - DEBOUNCE_TIME ) >= btn->lastPress ) {
        btn->lastPress = ms;
        btn->pressed = true;
    }    
}

//=============================================================================
//  Function to change the RGB LED color.
//-----------------------------------------------------------------------------
//  Arguments:
//    led   - LED number to change [ 0..3 ]
//    color - Color to set LED to  [ 0..7 ]
//-----------------------------------------------------------------------------
//  There is no range checking 
//=============================================================================
void setColor( uint8_t led, uint8_t color ) {
    int base = led * 3;
    digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ));
    mcp.digitalWrite( base + LED_RED, ( color & RED ) ); 
    mcp.digitalWrite( base + LED_GRN, ( color & GRN ) ); 
    mcp.digitalWrite( base + LED_BLU, ( color & BLU ) ); 
}

//===========================================================================
//  Setup 
//===========================================================================
void setup() {
    Serial.begin( SERIAL_SPEED );
    while (!Serial) { delay( SHORT_DELAY ); }  // Some boards need this delay

    pinMode( LED_BUILTIN, OUTPUT );
    pinMode( INTA, INPUT_PULLUP );  // Int is opendrain, so we need to pull it up

    // Install ISR for Pad Enable Buttons
    attachInterruptArg( digitalPinToInterrupt(INTA), isrButton, &button, FALLING );

    // Start mcp23017 Port Expander on I2C
    Wire.begin( I2C_SDA, I2C_SCL );
    mcp.begin( Wire );

    // Setup LED pins as output and initialize to off
    for ( uint8_t pin=0; pin < 12 ; pin++ ) {
	      mcp.pinMode( pin, OUTPUT ); 
        mcp.digitalWrite( pin, LOW );
	  }

    // Initialize MCP23XXX interrupts
    mcp.interruptSetup( MCP23XXX_MIRROR, MCP23XXX_OPENDRAIN, LOW );

    // Setup the buttons as INPUT_PULLUP
    mcp.pinMode( BUTTON_A, INPUT_PULLUP );
    mcp.pinMode( BUTTON_B, INPUT_PULLUP );
    mcp.pinMode( BUTTON_C, INPUT_PULLUP );
    mcp.pinMode( BUTTON_D, INPUT_PULLUP );

    // Enable interrupts on them when pressed (Falling)
    mcp.enableInterruptPin( BUTTON_A, FALLING );
    mcp.enableInterruptPin( BUTTON_B, FALLING );
    mcp.enableInterruptPin( BUTTON_C, FALLING );
    mcp.enableInterruptPin( BUTTON_D, FALLING ); 

    mcp.clearInterrupts(); 
}

void loop() {
    static int8_t color[] = { 0, 0, 0, 0 };

    if ( button.pressed ) {
        button.pressed = false;
        uint8_t intPin = mcp.getLastInterruptPin();
        if ( intPin != MCP23X17_INT_ERR ) {
            uint8_t intVal = mcp.getLastInterruptValue();  // Clears the interrupt
            uint8_t led = ( intPin - BUTTON_A );
            color[ led ] = ( color[ led ] + 1 ) & 0b0111; 
            setColor( led , color[ led ] ); 
            Serial.printf( "%d %d %d %d\n", color[0],color[1],color[2],color[3] ); 
            while ( !mcp.digitalRead( intPin )) { delay( SHORT_DELAY ); }
        }
    }

    delay( QUARTER_SECOND );    
}