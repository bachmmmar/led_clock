#include <Arduino.h>
#include "utils.h"

String Utils::padding0(uint8_t num) {
    if (num > 9) {
        return String(num);
    } else {
        return "0" + String(num);
    }
}