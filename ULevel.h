#pragma once
class ULevel {
public:
    union {
        struct {
            char pad_entityList[0x30];
            class AActor** EntityList; // Offset 0x30: Pointer to a list of Actor Pointers
            int currentEntities;       // Offset 0x34: The count of entities
            int maxEntities;           // Offset 0x38: The allocated size
        };
    };
};