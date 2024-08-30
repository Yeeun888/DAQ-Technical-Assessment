//NOT FOR USE IN PRODUCTION
//Useful for having when debugging. Left here if needed when adding more features

#include <iostream>

#include "DbcUtil.h"
#include "LogUtil.h"

void DEBUG_print_can_signal(can_signal t) {
    printf("%s %d %d %d %d %f %d %d %d %s\n",
        t.name.c_str(), t.start_position, t.length, 
        t.little_endian, t.is_signed, t.scale, 
        t.offset, t.min, t.max,
        t.unit.c_str()
    );
}