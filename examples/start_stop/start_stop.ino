/**
 * @file start_stop.ino
 * @author Brett Lempereur
 *
 * Start and stop a Reglo-CPF digital pump connected on the serial port.
 */

#include <RegloCPF.h>

// Create a pump controller using the serial port, with the default address.
RegloCPF pump(&Serial, 1);

// Digital LED pin.
const uint8_t PIN_LED = 13;

/**
 * Start the pump, pause for a second, and stop the pump.
 */
void setup()
{

    // Initialise the LED to use for communication.
    pinMode(PIN_LED, OUTPUT);

    // Start serial communication.
    Serial.begin(9600);

    // Attempt to start the pump, if an error occurs display the light.
    if (pump.start() != REGLO_OK) {
        digitalWrite(PIN_LED, HIGH);
    } else {

        // Wait for a second.
        delay(1000);
        
        // Attempt to stop the pump, again displying the light if an error
        // occurs.
        if (pump.stop() != REGLO_OK) {
            digitalWrite(PIN_LED, HIGH);
        }

    }

}

/**
 * Do nothing here.
 */
void loop()
{
    delay(10);
}

