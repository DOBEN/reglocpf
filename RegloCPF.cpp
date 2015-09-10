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

int RegloCPF::get_flow_rate(int* mantisse, int* exponent) {
	this->clear_buffer();

	int __request_code = request(REQUEST_GET_FLOW_RATE, _address);
	if (__request_code != REGLO_OK) {
		return REGLO_INTERNAL_ERROR;
	}

	return read_float_from_pump(mantisse, exponent);
}

int RegloCPF::set_flow_rate(int* mantisse, int* exponent) {
	this->clear_buffer();

	if (*exponent > 9 || *exponent < -9) {
		return REGLO_OUT_OF_RANGE;
	}

	if (*mantisse > 9999 || *mantisse < 0) {
		return REGLO_OUT_OF_RANGE;
	}

	char exponent_prefix = (*exponent >= 0) ? '+' : '-';

	/*int buffer=-1;
	 while (buffer!= -1) {  // leeren von buffer; wichtig weil später die antwort erwartet wird
	 buffer = _stream->read();
	 }*/

	int __request_code = request(REQUEST_SET_FLOW_RATE, _address, *mantisse,
			exponent_prefix, abs(*exponent));
	if (__request_code != REGLO_OK) {
		return REGLO_INTERNAL_ERROR;
	}
	return read_float_and_confirm(mantisse, exponent);
}
int RegloCPF::read_float_from_pump(int* mantisse, int* exponent) {
	char input[9] = { -1, -1, -1, -1, -1, -1, -1, -1, -1 };
	while (input[0] == -1) {  // stream not available
		input[0] = _stream->read();
	}

	if (*input == '#') {
		return REGLO_ERROR;
	}

	int i = 1;
	while (i < 9) {
		while (input[i] == -1) {   // stream not available
			input[i] = _stream->read();
		}
		i++;
	}

	sscanf(input, "%dE%d\r\n", mantisse, exponent);

	return REGLO_OK;

}

int RegloCPF::read_float_and_confirm(int* mantisse, int* exponent) {
	int mantisse_new = 0;
	int exponent_new = 0;
	int error = read_float_from_pump(&mantisse_new, &exponent_new);
	if (error != REGLO_OK) {
		return error;
	}

	double values = mantisse_new * pow(10, exponent_new);
	double old = *mantisse * pow(10, *exponent);
	*mantisse = mantisse_new;
	*exponent = exponent_new;

	if (round(values * 10000) == round(old * 10000)) { // rounding due to floating point precision and possible inaccuracy of pump response
		return REGLO_OK;
	} else {
		return REGLO_BAD_RESPONSE; //REGLO_BAD_RESPONSE or value too big or too small for the pump
		//pump will set the highest or lowest possible value and therefore values!= old;
		//max flow rate value in datasheet was 180ml/min; min flow rate value in datasheet was 0.08ml/min; but depending on the hubvolume of the pump this can change
		//max flow rate value of our pump was 36 ml/min (manual test),  min flow rate value 0.8 ml/min, respectivily
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

char RegloCPF::read() {
	if (_stream->available()) {
		return _stream->read();
	}
	return -1;
}

void RegloCPF::clear_buffer() {
	while (_stream->read() != -1) {
		_stream->read();
	}
}

int RegloCPF::confirm() {
	char response = _stream->read();
	int time=0;

	while (response == -1 && time<100000) {  // stream not available
		time++;
		response = _stream->read();
	}


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

