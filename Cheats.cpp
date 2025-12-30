#include "pch.h"
#include "Cheats.h"
#include "ULevel.h"
#include "AActor.h" 
#include "APawn.h"

bool Cheats::GetModules() {
    /* Loads Engine.dll Base*/
    engineModule = (uintptr_t)GetModuleHandleA("Engine.dll");
    return engineModule != NULL;
}

bool Cheats::GetLocalPlayer() {
    /*
    Loads the Currently Controlled APawn as myPawn

    Returns False if the APawn fails to load from memory

    */
    if (!engineModule && !GetModules()) {
        std::cout << "[!] Failed to fetch module handles." << std::endl;
    }
    static uintptr_t GEngineAddr = engineModule + 0x004C6934;
    uintptr_t GEngine = *(uintptr_t*)GEngineAddr;
    if (!GEngine) {
        std::cout << "[!] Failed to fetch Game Engine pointer." << std::endl;
        return false;
    };

    uintptr_t pawnAddr = *(uintptr_t*)(GEngine + 0x38);
    if (!pawnAddr) return false; 
    this->myPawn = (APawn*)pawnAddr;
    return (myPawn != nullptr);
}

void Cheats::Start() {
    ULevel* Level = myPawn->Level;

    for (int i = 0; i < Level->currentEntities; i++) {
        APawn* ent = (APawn*)Level->EntityList[i];
        // If the ents not valid or me skip
        if (ent == nullptr || ent == myPawn) continue;
        // I dont care if they are dead
        if (ent->health <= 0) continue;

        std::cout << "[+] Ent: " << i << " has " << ent->health << " health." << std::endl;;
    }
}
