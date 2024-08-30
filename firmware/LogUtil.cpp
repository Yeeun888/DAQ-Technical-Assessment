#include <string>
#include <bit>
#include <unordered_map>

#include "LogUtil.h"
#include "DbcUtil.h"

//Function Headers
uint16_t swap_uint16( uint16_t val );
int16_t swap_int16( int16_t val ); 
uint32_t swap_uint32( uint32_t val );
int32_t swap_int32( int32_t val );
int64_t swap_int64( int64_t val );
uint64_t swap_uint64( uint64_t val );

std::unordered_map<int, uint64_t> bitmasks;
void STARTUP_initialize_bitmask() {
    bitmasks.insert(std::make_pair(4, 0xF));
    bitmasks.insert(std::make_pair(8, 0xFF));
    bitmasks.insert(std::make_pair(16, 0xFFFF));
    bitmasks.insert(std::make_pair(32, 0xFFFFFFFF));
}

void extract_raw_data(raw_data* data, char* log_str) {
    //Assume that "()# " are not part of a naming scheme or can_interface name

    data->unix_time = strtok(log_str, "()# ");
    data->can_interface = strtok(nullptr, "()# ");
    data->id = std::stoull(strtok(nullptr, "()# "), 0, 16);

    char* s = strtok(nullptr, "()# ");

    //strtok produces ascii character 10 (backspace) where it detects nothing?
    //werid behavior when interacting with dump.log
    if(*s == 10) {
        data->data = 0;
        return;
    }

    data->data = std::stoull(s, 0, 16);
}

int raw_can_to_output(raw_data* raw_data, FILE* file, can_instruction* can_instruction) {
    u_int64_t little_endian_data = swap_uint64(raw_data->data);

    for(int i = 0; i < can_instruction->signals.size(); ++i) {
        u_int64_t data = raw_data->data;
        can_signal& curr_can_sig = can_instruction->signals[i];

        if(curr_can_sig.little_endian) {
            data = little_endian_data;
        }

        u_int64_t final_bitmask = bitmasks.at(curr_can_sig.length);
        u_int64_t masked_out_paylod = ((final_bitmask << curr_can_sig.start_position) & data) >> curr_can_sig.start_position;
        float final_payload = masked_out_paylod * curr_can_sig.scale + curr_can_sig.offset;
        
        fprintf(file, "(%s): %s: %.1f\n", raw_data->unix_time.c_str(), curr_can_sig.name.c_str(), final_payload);
    }
}

//Taken from 
//Credit to https://stackoverflow.com/questions/2182002/
//how-to-convert-big-endian-to-little-endian-in-c-without-using-library-functions
uint16_t swap_uint16( uint16_t val ) 
{
    return (val << 8) | (val >> 8 );
}

//! Byte swap short
int16_t swap_int16( int16_t val ) 
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t swap_uint32( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

int64_t swap_int64( int64_t val )
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

uint64_t swap_uint64( uint64_t val )
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}
