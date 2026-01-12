#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include <d3d9.h>
#include "MinHook/MinHook.h"
#include <algorithm>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "libMinHook.x86.lib") 

class AActor;
class APawn;
class ULevel;
class AController;

class Cheats {
private:
    uintptr_t engineModule = 0;
    uintptr_t coreModule = 0;
    IDirect3DDevice9* pDummyDevice = nullptr;
    IDirect3D9* pD3D = nullptr;
    HWND window = NULL;
    void* endSceneAddress = nullptr;

public:
    APawn* myPawn = nullptr;
    AController* myController = nullptr;
    bool bMenuOpen = true;
    bool bInstaKill = false;
    float fZedScaleValue = 1.0f;
    HWND gameWindow = NULL;
    bool bCanUnload = false;
    bool bGodMode = true;
    float* pTimescale;
    float* pGravity;
    float timescale;
    float gravity;

    void DrawMenu();
    bool FetchEndSceneAddress();
    bool CreateHook();
    void ReleaseDevice();
    std::vector<APawn*> GetMovingPawns();
    APawn* GetClosestEnemy(std::vector<APawn*> pawns);
    void TargetEntity(APawn* target);
    void InstaKill(std::vector<APawn*> pawns);
    bool GetModules();
    bool GetLocalPlayer();
    void Start();
    void RunCheats();
    void Cleanup();
    void ScaleAPawns(std::vector<APawn*> pawns);
    void DrawCrosshair();
    bool GetGravityPointer();
    bool GetTimescalePointer();
    bool SetGravity();
    bool SetTimescale();
};

extern Cheats cheats;