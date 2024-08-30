/**
 * Author : Maximus Philips Chan (z5457349)
 * 
 * Built for Redback Racing DAQ Technical Assessment
 */

//Cpp library dependencies
#include <bit>
#include <iostream>

//File dependencies
#include "DbcUtil.h"
#include "LogUtil.h"

#define BUFFER_SIZE 64

bool exists_in_map(u_int32_t can_id, std::unordered_map<u_int32_t, can_instruction>* can_signal_instruction); 

/**
 * @brief main
 * 
 * @return EXIT CODE 0 SUCESS, 1 PROBLEM ENCOUNTERED.
 */
int main() {
    //Start Up Required Functions 
    STARTUP_initialize_bitmask();

    //Initialize both files for reading
    FILE* dbc_file = fopen("SensorBus.dbc", "r");
    if(dbc_file == nullptr) {
        std::cout << "DBC not found in directory / does not exist\n";
        return 1;
    }
    FILE* log_file = fopen("dump.log", "r");
    if(log_file == nullptr) {
        std::cout << "LOG not found in directory / does not exist\n";
        return 1;
    }

    //Maps CAN ID to CAN Instructions on how to retrive and interpret CAN data
    std::unordered_map<u_int32_t, can_instruction>* can_signal_instruction = extract_dbc(dbc_file);

    //Setup Data for main while loop
    raw_data rd;                                    //Temporary raw data
    FILE* output_file = fopen("output.txt", "w");   //File to Write to. Overwrites old output.txt
    char buffer[BUFFER_SIZE];                       //fgets write buffer

    while(fgets(buffer, BUFFER_SIZE, log_file)) {
        extract_raw_data(&rd, buffer);

        if(exists_in_map(rd.id, can_signal_instruction)) {
            raw_can_to_output(&rd, output_file, &(can_signal_instruction->at(rd.id)));
        }
    }

    fclose(output_file);
    fclose(dbc_file);
    fclose(log_file);

    return 0;
}

/**
 * @brief For a given pointer to a std::unordered_map<u_int32_t, can_instruction> check if can_instruction
 * exists for integer
 * 
 * @param can_id can_id to look for
 * @param can_signal_instruction map for can_instruction
 * @return true if true
 * @return false if false
 */
bool exists_in_map(u_int32_t can_id, std::unordered_map<u_int32_t, can_instruction>* can_signal_instruction) {
    if(can_signal_instruction->find(can_id) != can_signal_instruction->end()) {
        return true;
    } else {
        return false;
    }
}