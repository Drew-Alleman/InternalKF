#pragma once
#include "AActor.h"
#include "APlayerController.h" 

class APawn : public AActor {
public:
    union {
        // --- Pawn Specific (Movement & Logic) ---
        struct { char pad_controller[0x360]; AController* Controller; };
    };
};