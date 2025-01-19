#include "usb_port.h"
#include <stdlib.h>
#include <stdio.h>

float read_temperature_from_usb() {
    float t = ((float)rand()/RAND_MAX)*(float)(50.0);
    return t;
}