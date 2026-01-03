#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include <d3d9.h>
#include "MinHook/MinHook.h" 
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "libMinHook.x86.lib") 

class AActor;
class APawn;
class ULevel;

class Cheats {
private:
    uintptr_t engineModule = 0;
    IDirect3DDevice9* pDummyDevice = nullptr;
    IDirect3D9* pD3D = nullptr;
    HWND window = NULL;
    void* endSceneAddress = nullptr;

public:
    APawn* myPawn = nullptr;
    bool bMenuOpen = true;
    bool bInstaKill = false;
    float fZedScaleValue = 1.0f;
    HWND gameWindow = NULL;
    bool bCanUnload = false;
    bool bGodMode = true;

    void DrawMenu();
    bool FetchEndSceneAddress();
    bool CreateHook();
    void ReleaseDevice();
    std::vector<APawn*> GetMovingPawns();
    void InstaKill();
    bool GetModules();
    bool GetLocalPlayer();
    void Start();
    void Cleanup();
    void ScaleAActorsUp();
    void ScaleAActorsDown();
};

extern Cheats cheats;