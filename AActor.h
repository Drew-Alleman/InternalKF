#pragma once
#include "Enums.h"

class AActor {
public:
    union {

        struct {
            char pad_brush[0x40];
            bool isBrush;
        };

        struct {
            char pad_rot[0x158];
            int Pitch; // 0x158
            int Yaw;   // 0x15C
            int Roll;  // 0x160
        };

        struct {
            char pad_3dSize[0x264];
            float x3DDrawScale;
            float y3DDrawScale;
            float z3DDrawScale;
        };

        struct {
            char pad_x[0x448];
            float height;
        };

        struct {
            char pad_drawSize[0x260];
            float drawSize;
        };

        struct {
            char pad_physics[0x74];
            PHYS physics;
        };

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
            float y;                     // 0x14C 
            float x;                     // 0x150
            float z;                     // 0x154
        };


    };
};
