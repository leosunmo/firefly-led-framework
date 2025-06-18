#pragma once

namespace FireFly {

/**
 * Enumeration of pattern types with fixed IDs
 * Each pattern has a unique ID that can be used for direct selection
 * 
 * IMPORTANT: This enum must match the UART message pattern payload enum
 * used by the device sending UART messages to control the LED framework
 */
enum class PatternType : uint8_t {
    // Basic patterns (0-9)
    NONE = 0,
    CHROMA_WAVE = 1,
    SHAKEEL_FLASH = 2,
    SHAKEEL_FLASH_BALL = 3,
    
    // Fire-themed patterns (10-19)
    FIRE_FLIES = 10,
    FIRE_FLIES_SAME = 11,
    FIRE_V0 = 12,
    FIRE_V0_MIRROR = 13,
    
    // Sound reactive patterns (20-39)
    SHAKEEL = 20,
    // SHAKEEL_FLASH - moved to ID 2
    // SHAKEEL_FLASH_BALL - moved to ID 3
    BOUNCE = 23,
    BOUNCE_SPLIT = 24,
    BOUNCE_HIGH_LOW = 25,
    SPACE_X = 26,
    HEART = 27,
    CIRCLES = 28,
    RAINDROP = 29,
    // CHROMA_WAVE - moved to ID 1
    
    // Special patterns (40-49)
    WIRELESS = 40,
    
    // Add new pattern types here with their IDs
    
    // Special values
    COUNT,      // Total number of pattern types (not a valid pattern)
    DEFAULT = SHAKEEL_FLASH  // Default pattern to use
};

/**
 * Convert a uint8_t to a PatternType
 * @param value The uint8_t value
 * @return The PatternType or DEFAULT if the value doesn't match any defined pattern
 */
inline PatternType toPatternType(uint8_t value) {
    if (value >= static_cast<uint8_t>(PatternType::COUNT)) {
        return PatternType::DEFAULT;
    }
    return static_cast<PatternType>(value);
}

/**
 * Get a string representation of a PatternType
 * @param type The PatternType
 * @return A string describing the pattern
 */
inline const char* getPatternName(PatternType type) {
    switch (type) {
        case PatternType::NONE:             return "None";
        case PatternType::FIRE_FLIES:       return "Fire Flies";
        case PatternType::FIRE_FLIES_SAME:  return "Fire Flies Same";
        case PatternType::FIRE_V0:          return "Fire V0";
        case PatternType::FIRE_V0_MIRROR:   return "Fire V0 Mirror";
        case PatternType::SHAKEEL:          return "Shakeel";
        case PatternType::SHAKEEL_FLASH:    return "Shakeel Flash";
        case PatternType::SHAKEEL_FLASH_BALL: return "Shakeel Flash Ball";
        case PatternType::BOUNCE:           return "Bounce";
        case PatternType::BOUNCE_SPLIT:     return "Bounce Split";
        case PatternType::BOUNCE_HIGH_LOW:  return "Bounce High Low";
        case PatternType::SPACE_X:          return "Space X";
        case PatternType::HEART:            return "Heart";
        case PatternType::CIRCLES:          return "Circles";
        case PatternType::RAINDROP:         return "Raindrop";
        case PatternType::CHROMA_WAVE:      return "Chroma Wave";
        case PatternType::WIRELESS:         return "Wireless";
        default:                           return "Unknown";
    }
}

} // namespace FireFly
