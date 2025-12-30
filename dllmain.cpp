// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Cheats.h"
#include <windows.h>
#include <iostream>


void CreateConsole() {
    // open a command console findow
    AllocConsole();
    // init stout, stderr, and stdin
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONIN$", "r", stdin);
    std::cout.clear();
    // Now we have output!
    std::cout << "[+] Console Allocated Successfully!" << std::endl;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;
    CreateConsole();
    Cheats c = Cheats();
    if (!c.GetLocalPlayer()) {
        std::cout << "Failed to fetch current player!" << std::endl;
    }
    else {
        c.Start();
    }
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
