#include "logger.h"
#include <Arduino.h>

//#define Serial SERIAL_PORT_USBVIRTUAL

Logger::Logger() :
        current_severity_(DEBUG) {
}

void Logger::setup(void) {
    Serial.begin(9600);
}

void Logger::log(Severity severity, const String & message) {
    if (current_severity_ <= severity) {
        String str = String(String(millis()) + ";" + getSeverityAsString(severity) + ";" + message);
        Serial.println(str.c_str());
    }
}


String Logger::getSeverityAsString(Severity severity) const {
    switch (severity) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case ERROR:
            return "ERROR";
    }
    return "UNDEF";
}

void Logger::setSeverity(Severity severity) {
    current_severity_ = severity;
    log(INFO, "Logging severity changed to: " + getSeverityAsString(severity));
}
