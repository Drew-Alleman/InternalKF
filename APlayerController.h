#pragma once
#include "AActor.h"

// AController is a type of Actor
class AController : public AActor {
public:
    // It inherits Pitch/Yaw/Roll from AActor automatically!
    union {
        // Add Controller-specific offsets here later if needed
    };
};