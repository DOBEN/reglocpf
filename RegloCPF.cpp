/**
 * @file RegloCPF.h
 * @author Brett Lempereur
 *
 * Interface for controlling a Reglo-CPF digital pump.
 */

#include "RegloCPF.h"

// Command requests.
const char* REQUEST_START = "%dH\r";
const char* REQUEST_STOP = "%dI\r";
const char* REQUEST_DISABLE_CONTROL_PANEL = "%dB\r";
const char* REQUEST_ENABLE_CONTROL_PANEL = "%dA\r";
const char* REQUEST_CLOCKWISE = "%dJ\r";
const char* REQUEST_COUNTER_CLOCKWISE = "%dK\r";
const char* REQUEST_GET_FLOW_RATE = "%df\r";
const char* REQUEST_SET_FLOW_RATE = "%df%.4d%c%.1d\r";

// Buffer size for command formatting.
const int BUFFER_SIZE = 16;

// Command response codes.
const char RESPONSE_OK = '*';
const char RESPONSE_ERROR = '#';
const char RESPONSE_TIMEOUT = -1;

// Common request and confirm pattern as a macro.
#define REQUEST_AND_CONFIRM(command, ...) { \
        int __request_code = request(command, ##__VA_ARGS__); \
        if (__request_code != REGLO_OK) return __request_code; \
        return confirm(); \
    }

RegloCPF::RegloCPF(Stream* stream, const uint8_t address) {
	_stream = stream;
	_address = address;
}

int RegloCPF::start() {
	REQUEST_AND_CONFIRM(REQUEST_START, _address);
}

int RegloCPF::stop() {
	REQUEST_AND_CONFIRM(REQUEST_STOP, _address);
}

int RegloCPF::disable_control_panel() {
	REQUEST_AND_CONFIRM(REQUEST_DISABLE_CONTROL_PANEL, _address);
}

int RegloCPF::enable_control_panel() {
	REQUEST_AND_CONFIRM(REQUEST_ENABLE_CONTROL_PANEL, _address);
}

int RegloCPF::clockwise() {
	REQUEST_AND_CONFIRM(REQUEST_CLOCKWISE, _address);
}

int RegloCPF::counterClockwise() {
	REQUEST_AND_CONFIRM(REQUEST_COUNTER_CLOCKWISE, _address);
}

String RegloCPF::get_flow_rate() {
	int __request_code = request(REQUEST_GET_FLOW_RATE, _address);
	if (__request_code != REGLO_OK) {
		return "REGLO_INTERNAL_ERROR";
	}
	String response = _stream->readString();
	return response;
}

int RegloCPF::set_flow_rate(int* mantisse, int* exponent) {

	int mantisse_new = 0;
	int exponent_new = 0;
	char input[7] = { -1, -1, -1, -1, -1, -1, -1 };

	if (*exponent > 9 || *exponent < -9) {
		return REGLO_OUT_OF_RANGE;
	}

	if (*mantisse > 9999 || *mantisse < 0) {
		return REGLO_OUT_OF_RANGE;
	}

	char exponent_prefix = (*exponent >= 0) ? '+' : '-';

	int __request_code = request(REQUEST_SET_FLOW_RATE, _address, *mantisse,
			exponent_prefix, abs(*exponent));
	if (__request_code != REGLO_OK) {
		return REGLO_INTERNAL_ERROR;
	}

	while (input[0] == -1) {  // stream not available
		input[0] = _stream->read();
	}

	if (*input == '#') {
		return REGLO_ERROR;
	}

	int i = 1;
	while (i < 7) {
		while (input[i] == -1) {   // stream not available
			input[i] = _stream->read();
		}
		i++;
	}

	sscanf(input, "%dE%d\r\n", &mantisse_new, &exponent_new);

	double values = mantisse_new * pow(10, exponent_new);
	double old = *mantisse * pow(10, *exponent);
	*mantisse = mantisse_new;
	*exponent = exponent_new;

	if (round(values * 10000) == round(old * 10000)) {// rounding due to floating point precision and possible inaccuracy of pump response
		return REGLO_OK;
	} else {
		return REGLO_BAD_RESPONSE; //REGLO_BAD_RESPONSE or value too big or too small for the pump
		//pump will set the highest or lowest possible value and therefore compare != response;
		//max value in datasheet was 180ml/min; min value in datasheet was 0.08ml/min; but depending on the hubvolume of the pump this can change
// max value manuell test 36 ml/min      min  0.8 ml/min
	}
}

int RegloCPF::request(const char* command, ...) {
	va_list args;
	char buffer[BUFFER_SIZE];

// Format the command from the variadic argument list.
	va_start(args, command);
	int result = vsnprintf(buffer, BUFFER_SIZE, command, args);
	va_end(args);

// If the command was malformed or could not fit in the buffer, fail fast.
	if (result == -1 || result >= BUFFER_SIZE) {
		return REGLO_INTERNAL_ERROR;
	}

// Send the command to the pump.
	_stream->print(buffer);
	return REGLO_OK;
}

int RegloCPF::confirm() {
	char response = _stream->read();

	switch (response) {
	case RESPONSE_OK:
		return REGLO_OK;
	case RESPONSE_ERROR:
		return REGLO_ERROR;
	case RESPONSE_TIMEOUT:
		return REGLO_TIMEOUT;
	default:
		return REGLO_BAD_RESPONSE;
	}
}

