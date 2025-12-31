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

std::vector<AActor*> Cheats::GetMovingAActors() {
    std::vector<AActor*> AActors;
    ULevel* Level = myPawn->Level;
    if (!Level) return AActors;
    for (int i = 0; i < Level->currentEntities; i++) {

        AActor* genericActor = Level->EntityList[i];
        if (!genericActor || genericActor == (AActor*)myPawn) continue;

        if (genericActor->physics != PHYS::Walking && genericActor->physics != PHYS::Falling) continue;


        APawn* ent = (APawn*)genericActor;

        if (ent->health <= 0 || ent->health > 10000) continue;

        AActors.push_back(ent);
        //std::cout << "[+] Found Entity [" << i << "] Health: " << ent->health << std::endl;
    }
    return AActors;
}

void Cheats::ScaleAActorsUp() {
    std::vector<AActor*> AActors = GetMovingAActors();
    if (AActors.empty()) {
        std::cout << "[!] No actors to scale up in map!" << std::endl;
        return;
    }

    for (AActor* actor : AActors) {
        actor->x3DDrawScale = actor->x3DDrawScale * 2;
        actor->y3DDrawScale = actor->y3DDrawScale * 2;
        actor->z3DDrawScale = actor->z3DDrawScale * 2;
        actor->drawSize = actor->drawSize * 2;
    }
}

void Cheats::ScaleAActorsDown() {
    std::vector<AActor*> AActors = GetMovingAActors();
    if (AActors.empty()) {
        std::cout << "[!] No actors to scale down in map!" << std::endl;
        return;
    }

    for (AActor* actor : AActors) {
        actor->x3DDrawScale /= 2.0f;
        actor->y3DDrawScale /= 2.0f;
        actor->z3DDrawScale /= 2.0f;

        actor->drawSize /= 2.0f;

        if (actor->drawSize < 0.1f) actor->drawSize = 0.1f;
    }
    std::cout << "[+] Scaled " << AActors.size() << " Zeds down to mini-mode!" << std::endl;
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

void Cheats::InstaKill() {
    std::vector<AActor*> AActors = GetMovingAActors();
    if (AActors.empty()) {
        std::cout << "[!] No actors to instakill in map!" << std::endl;
        return;
    }

    for (AActor* actor : AActors) {
        // If the actor is not us!
        if (actor == myPawn) {
            continue;
        }
        if (actor->health != 1) {
            actor->health = 1;
        }
        std::cout << "[!] Set Entity to 1 HP!" << std::endl;
    }

}

void Cheats::Start() {

    while (!GetAsyncKeyState(VK_END) & 1) {
        if (!GetLocalPlayer()) {
            std::cout << "[!] Failed to get APawn waiting 5 seconds" << std::endl;
            Sleep(5000);
            continue;

        }

        if (myPawn->health <= 0) {
            std::cout << "[!] Player is dead; sleeping for 3 seconds" << std::endl;
            Sleep(3000);
            continue;
        }

        if (GetAsyncKeyState(VK_F1) & 1) {
            InstaKill();
        }

        if (GetAsyncKeyState(VK_F2) & 1) {
            ScaleAActorsUp();
        }

        if (GetAsyncKeyState(VK_F3) & 1) {
            ScaleAActorsDown();
        }

        Sleep(50);
    }
}