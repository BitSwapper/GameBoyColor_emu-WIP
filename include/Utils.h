#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <sstream>   
#include <iomanip>   
#include <cstdint>   


inline std::string formatHexVal(uint16_t val, int width, bool prefix) {
    std::ostringstream oss;
    if (prefix) oss << "0x";
    oss << std::hex << std::uppercase << std::setw(width) << std::setfill('0') << static_cast<int>(val); 
    return oss.str();
}

inline std::string formatHex16(uint16_t val, bool prefix = true) {
    return formatHexVal(val, 4, prefix);
}

inline std::string formatHex8(uint8_t val, bool prefix = true) {
    return formatHexVal(val, 2, prefix);
}

#endif 