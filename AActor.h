#pragma once
class AActor {
public:
    union {

        struct {
            char pad_008[0x480];
            int health;
        };

        struct {
            char pad_level[0x9c];  // New padding to reach 0x9c
            class ULevel* Level;
        };

        struct {
            char pad_003[0x14C];
            float y;                     // 0x14C (Based on your CE table)
            float x;                     // 0x150
            float z;                     // 0x154
        };
    };
};
