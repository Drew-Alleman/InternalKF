#pragma once
class Weapon {
public:
    union {
        // --- Visuals
        struct {
            char pad_scale[0x260];
            float weaponWidth;   // 0x260
            float weaponHeight;  // 0x264
            float weaponDepth;   // 0x268
            float weaponScale;   // 0x26C
        };

        struct {
            char pad_zoom[0x514];
            float weaponZoom;    // 0x514
        };

        struct {
            char pad_reload[0x5B0];
            float reloadSpeed;   // 0x5B0
        };
    };
};