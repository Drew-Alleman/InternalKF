#include "pch.h"
#include "Cheats.h"
#include "ULevel.h"
#include "AActor.h" 
#include "APawn.h"
#include "Enums.h"

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
    if (!Level) return;

    for (int i = 0; i < Level->currentEntities; i++) {

        AActor* genericActor = Level->EntityList[i];
        if (!genericActor || genericActor == (AActor*)myPawn) continue;

        if (genericActor->physics != PHYS::Walking && genericActor->physics != PHYS::Falling) continue;


        APawn* ent = (APawn*)genericActor;

        if (ent->health <= 0 || ent->health > 10000) continue;

        std::cout << "[+] Found Entity [" << i << "] Health: " << ent->health << std::endl;
    }
}