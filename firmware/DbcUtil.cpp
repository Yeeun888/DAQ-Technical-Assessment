#include <stdint.h>
#include <vector>
#include <unordered_map>

#include "DbcUtil.h"

void inline instruction_add_can_signal(can_instruction* dbc, can_signal* signal) {
    dbc->signals.push_back(*signal); 
}

std::unordered_map<u_int32_t, can_instruction>* extract_dbc(FILE* file) {
    
    std::unordered_map<u_int32_t, can_instruction>* map = new std::unordered_map<u_int32_t, can_instruction>(); 

    //Hard coded here but can be modified later for general DBC files
    //Based on SensorBus.dbc
    
    //START of hard coding
    can_instruction instruction = can_instruction(1797);

    can_signal sig3 = can_signal("WheelSpeedFR", 0, 16, true, false, 0.1, 0, 0, 0, "km/h");
    // can_signal sig4 = can_signal("WheelSpeedFL", 16, 16, true, false, 0.1, 0, 0, 0, "km/h");
    can_signal sig1 = can_signal("WheelSpeedRR", 32, 16, true, false, 0.1, 0, 0, 0, "km/h");
    // can_signal sig2 = can_signal("WheelSpeedRL", 48, 16, true, false, 0.1, 0, 0, 0, "km/h");

    instruction_add_can_signal(&instruction, &sig3);
    // instruction_add_can_signal(&instruction, &sig4);
    instruction_add_can_signal(&instruction, &sig1);
    // instruction_add_can_signal(&instruction, &sig2);

    map->insert(std::make_pair(1797, instruction));
    //END of hard coding

    return map;
}
