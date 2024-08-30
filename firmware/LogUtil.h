#include <string>

#include "DbcUtil.h"

#ifndef LOGUTIL
#define LOGUTIL

/**
 * @brief Struct for raw data from a log file. 
 * 
 */
struct raw_data {
    std::string unix_time;
    std::string can_interface;
    uint_fast32_t id;
    uint_fast64_t data;
};

/**
 * @brief Given a line of log can data modifies given raw_data struct and returns 
 * extracted versions.
 * 
 * @pre id and data must be of hexadecimal form
 * 
 * @param data 
 * @param log_str 
 */
void extract_raw_data(raw_data* data, char* log_str);

/**
 * @brief 
 * 
 * @param data 
 * @param file 
 * @param can_instruction 
 * @return int 
 */
int raw_can_to_output(raw_data* data, FILE* file, can_instruction* can_instruction);

/**
 * @brief Required function as std::unordered_map list initialization does not work on all
 * version of C++. Function as part of main. 
 */
void STARTUP_initialize_bitmask();

#endif