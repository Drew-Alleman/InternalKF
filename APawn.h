#pragma once
#include "AActor.h"
#include "APlayerController.h" 

class APawn : public AActor {
public:
    union {
        struct {
            char pad_controller[0x360];
            AController* Controller; // 0x360
        };
    };
};