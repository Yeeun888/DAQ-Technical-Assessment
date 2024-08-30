#include <string>
#include <vector>
#include <unordered_map>

#ifndef DBCUTILS
#define DBCUTILS

/**
 * @brief Can signal_form. Contains all the data for a given can_instruction on
 * how to extract information. Interpret the data using start_position and 
 * little_endianness and continue with other variables. 
 */
struct can_signal {
    std::string name;
    int start_position;
    int length;
    bool little_endian;
    bool is_signed;
    float scale;
    int offset;
    int min;
    int max;
    std::string unit;

    //Default constructor based on read data
    can_signal(
            std::string name_,
            int start_position_,
            int length_,
            bool little_endian_,
            bool is_signed_,
            float scale_,
            int offset_,
            int min_,
            int max_, 
            std::string unit_
        ) {

        name = name_;
        start_position = start_position_;
        length = length_;
        little_endian = little_endian_;
        is_signed = is_signed_;
        scale = scale_;
        offset = offset_;
        min = min_;
        max = max_;
        unit = unit_;
    }
};

/**
 * @brief Template for a single signal. A DBC file with multiple signals can have
 * multiple can_instructions
 * 
 */
struct can_instruction {
    uint_fast32_t can_id;

    //Extend as required upto size 64. Above 64 should result in undefined behavior
    //in dbcUtil code
    std::vector<struct can_signal> signals;

    can_instruction(uint_fast32_t can_id_) {
        can_id = can_id_;
    }
};

/**
 * @brief Generates an unordered map from id to a can_instruction. For each ID 
 * in log, map allows O(1) instruction access performance
 * 
 * @param file 
 * @return std::unordered_map<u_int32_t, can_instruction>* 
 */
std::unordered_map<u_int32_t, can_instruction>* extract_dbc(FILE* file);

#endif