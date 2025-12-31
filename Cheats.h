#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>

// Forward Declarations: Tells the compiler these exist without loading the files yet
class AActor;
class APlayerController;
class APawn;
class ULevel;

class Cheats {
private:
    bool keepRunning = true;
    uintptr_t engineModule = 0;

public:
    APawn* myPawn = nullptr;
    std::vector<AActor*> GetMovingAActors();
    void InstaKill();
    bool GetModules();
    bool GetLocalPlayer();
    void Start();
    void ScaleAActorsUp();
    void ScaleAActorsDown();
};