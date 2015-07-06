/**
 * @file RegloCPF.h
 * @author Brett Lempereur
 *
 * Interface for controlling a Reglo-CPF digital pump.
 */

#include <stdarg.h>
#include <stdio.h>

#include "RegloCPF.h"

// Command requests.
const char* REQUEST_START = "%dH\r";
const char* REQUEST_STOP = "%dI\r";
const char* REQUEST_CLOCKWISE = "%dJ\r";
const char* REQUEST_COUNTER_CLOCKWISE = "%dK\r";

// Buffer size for command formatting.
const int BUFFER_SIZE = 16;

// Command response codes.
const char RESPONSE_OK = '*';
const char RESPONSE_ERROR = '#';
const char RESPONSE_TIMEOUT = -1;

// Common request and confirm pattern as a macro.
#define REQUEST_AND_CONFIRM(command, ...) { \
        int __request_code = request(command, ##__VA_ARGS__); \
        if (__request_code != RESPONSE_OK) return __request_code; \
        return confirm(); \
    }

RegloCPF::RegloCPF(Stream* stream, const uint8_t address)
{
    _stream = stream;
    _address = address;
}

int RegloCPF::start()
{
    REQUEST_AND_CONFIRM(REQUEST_START, _address);
}

int RegloCPF::stop()
{
    REQUEST_AND_CONFIRM(REQUEST_STOP, _address);
}

int RegloCPF::clockwise()
{
    REQUEST_AND_CONFIRM(REQUEST_CLOCKWISE, _address);
}

int RegloCPF::counterClockwise()
{
    REQUEST_AND_CONFIRM(REQUEST_COUNTER_CLOCKWISE, _address);
}

int RegloCPF::request(const char* command, ...)
{
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

int RegloCPF::confirm()
{
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

