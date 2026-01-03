// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Cheats.h"
#include <windows.h>
#include <iostream>


void CreateConsole() {
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
    std::cout.clear();
    std::cout << "[+] Console Allocated Successfully!" << std::endl;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;
    CreateConsole();

    if (!cheats.GetLocalPlayer()) {
        std::cout << "Failed to fetch current player!" << std::endl;
    }

    cheats.Start();

    std::cout << "[*] Cleaning up and Detaching..." << std::endl;
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {

    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        return TRUE;
    }
    return FALSE;
}
