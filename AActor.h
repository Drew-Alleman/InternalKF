#pragma once
#include "Enums.h"

class Weapon;

class AActor {
public:
    // Using a single union in the base class ensures all offsets are 
    // calculated from the same starting address (0x0)
    union {
        // --- Core Engine (0x00 - 0x100) ---
        struct { char pad_brush[0x40]; bool isBrush; };
        struct { char pad_physics[0x74]; PHYS physics; };
        struct { char pad_level[0x9C]; ULevel* Level; };

        // --- Transformation (0x100 - 0x200) ---
        struct { char pad_loc[0x14C]; float y, x, z; };
        struct { char pad_rot[0x158]; int Pitch, Yaw, Roll; };

        // --- Rendering (0x200 - 0x300) ---
        struct { char pad_collision[0x2c0]; float collisionHeight, collisionRadius; };
        struct { char pad_drawSize[0x260]; float drawScale; };
        struct { char pad_3dSize[0x264]; float x3DDrawScale, y3DDrawScale, z3DDrawScale; };
        struct { char pad_glow[0x28C]; unsigned char glow; };

        // --- Controller & Interaction (0x300 - 0x400) ---
        struct { char pad_controller[0x360]; class AController* Controller; };
        struct { char pad_zjump[0x3F8]; float zJumpHeight; };

        // --- Health & Pawn Data (0x400 - 0x500) ---
        struct { char pad_weapon[0x43C]; Weapon* currentWeapon; };
        struct { char pad_eyes[0x448]; float baseEyeHeight, eyeHeight; };
        struct { char pad_superMax[0x47C]; float superHealthMax; };
        struct { char pad_hp[0x480]; int health; };
        struct { char pad_head[0x494]; float headRadius, headHeight, headScale; };

        // --- Movement (0xE00+) ---
        struct { char pad_speed[0xE70]; float speed; };
    };
};