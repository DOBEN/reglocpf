/**
 * @file RegloCPF.h
 * @author Brett Lempereur
 *
 * Interface for controlling a Reglo-CPF digital pump.
 */

#ifndef REGLO_CPF_H
#define REGLO_CPF_H

#include <Stream.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

/**
 * Return codes for pump control commands.
 */
enum {
	REGLO_OK,               //!< Command successful.
	REGLO_ERROR,            //!< Command failed.
	REGLO_TIMEOUT,          //!< Response not received in time.
	REGLO_OUT_OF_RANGE,     //!< Parameter is not within safe range.
	REGLO_INTERNAL_ERROR,   //!< Internal error in the control interface.
	REGLO_BAD_RESPONSE      //!< Unknown response from pump.
};

/**
 * Reglo-CPF pump control interface.
 */
class RegloCPF {

	Stream* _stream;
	uint8_t _address;

	/**
	 * Issue a request to the digital pump.
	 *
	 * @param[in] command   Command string.
	 * @param[in] ...       Command parameters.
	 */
	int request(const char* command, ...);

	/**
	 * Return the response to a command that succeeds or fails.
	 */
	int confirm();

public:

	/**
	 * Construct a new RegloCPF controller.
	 *
	 * @param[in] stream    Controller communication stream, typically Serial.
	 * @param[in] address   Controller address, in range 1 to 8.
	 */
	RegloCPF(Stream* stream, const uint8_t address);

	/**
	 * Start the pump.
	 */
	int start();

	/**
	 * Stop the pump.
	 */
	int stop();

	/**
	 * Set revolution in clockwise direction.
	 */
	int clockwise();

	/**
	 * Set revolution in counter-clockwise direction.
	 */
	int counterClockwise();


	/**
	 * Set control panel inactive.
	 */
	int disable_control_panel();

	/**
	 * Switch control panel to manual operation.
	 */
	int enable_control_panel();

	/**
	 * Flow rate in ml per minute
	 */
	String get_flow_rate();

	/**
	 * Set flow rate in ml per minute; first value is Mantisse; second value is Exponent e.g. -2 or 7
	 */
	int set_flow_rate(int* mantisse, int* exponent);

};

#endif

