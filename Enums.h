#pragma once

enum class PHYS : uint8_t {
    None = 0,
    Walking = 1,
    Falling = 2,
    Unknown = 3,
    Flying = 4,
    Freeze = 5,
};

inline const char* GetPhysicsName(unsigned char physics) {
    switch (physics) {
    case 0: return "PHYS_None";
    case 1: return "PHYS_Walking";
    case 2: return "PHYS_Falling";
    case 3: return "PHYS_UNKNOWN";
    case 4: return "PHYS_FLYING";
    case 5: return "PHYS_FREEZE";
    default: return "PHYS_Unknown";
    }
}
