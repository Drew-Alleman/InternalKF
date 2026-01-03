#include "pch.h"
#include "Cheats.h"
#include "ULevel.h"
#include "AActor.h" 
#include "APawn.h"
#include "Enums.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WNDPROC oWndProc;
typedef HRESULT(STDMETHODCALLTYPE* EndScene_t)(IDirect3DDevice9*);
EndScene_t oEndScene = nullptr;

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // --- Always allow your global hotkeys (even when menu is closed) ---
    if (uMsg == WM_KEYDOWN)
    {
        switch (wParam)
        {
        case VK_F1: cheats.bInstaKill = !cheats.bInstaKill; return 0;
        case VK_F2: cheats.bGodMode = !cheats.bGodMode;   return 0;
        case VK_F3: cheats.ScaleAActorsUp();               return 0;
        case VK_F4: cheats.ScaleAActorsDown();             return 0;

        case VK_INSERT:
            cheats.bMenuOpen = !cheats.bMenuOpen;
            if (ImGui::GetCurrentContext())
                // When true, ImGui renders its own mouse cursor
                ImGui::GetIO().MouseDrawCursor = cheats.bMenuOpen;
            return 0;

        case VK_END:
            cheats.bCanUnload = true;
            return 0;
        }
    }

    // If menu is closed, do NOT feed ImGui. Just pass to the game.
    if (!cheats.bMenuOpen)
        return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);

    // Menu is open: feed everything to ImGui first
    if (ImGui::GetCurrentContext())
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

    // Handle ImGui input
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        return 0; // swallow input, don't send to game

    // Otherwise, let the game have it
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice9* pDevice) {
    if (cheats.bCanUnload) return oEndScene(pDevice);

    static bool init = false;
    if (!init) {
        D3DDEVICE_CREATION_PARAMETERS params;
        pDevice->GetCreationParameters(&params);
        cheats.gameWindow = params.hFocusWindow;

        oWndProc = (WNDPROC)SetWindowLongPtr(cheats.gameWindow, GWLP_WNDPROC, (LONG_PTR)WndProc);

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(cheats.gameWindow);
        ImGui_ImplDX9_Init(pDevice);

        init = true;
    }

    // 1. BACKUP GAME STATE
    // This prevents ImGui from messing up the game's textures/lighting
    IDirect3DStateBlock9* pStateBlock = nullptr;
    if (pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock) == D3D_OK) {
        pStateBlock->Capture();
    }

    // 2. RENDER IMGUI
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    cheats.DrawMenu();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    // 3. RESTORE GAME STATE & CLEANUP
    if (pStateBlock) {
        pStateBlock->Apply();
        pStateBlock->Release();
    }

    return oEndScene(pDevice);
}

bool Cheats::GetModules() {
    engineModule = (uintptr_t)GetModuleHandleA("Engine.dll");
    return engineModule != NULL;
}

bool Cheats::GetLocalPlayer() {
    if (!engineModule && !GetModules()) return false;

    static uintptr_t GEngineAddr = engineModule + 0x004C6934;
    uintptr_t GEngine = *(uintptr_t*)GEngineAddr;
    if (!GEngine) return false;

    uintptr_t pawnAddr = *(uintptr_t*)(GEngine + 0x38);
    if (!pawnAddr) return false;

    this->myPawn = (APawn*)pawnAddr;
    return (myPawn != nullptr);
}

std::vector<APawn*> Cheats::GetMovingPawns() {
    std::vector<APawn*> pawns;

    if (!myPawn || !myPawn->Level)
        return pawns;

    ULevel* level = myPawn->Level;

    // sanity clamp (prevents insane loops if currentEntities is wrong/stale)
    if (level->currentEntities <= 0 || level->currentEntities > 50000)
        return pawns;

    for (int i = 0; i < level->currentEntities; i++) {
        AActor* a = level->EntityList[i];
        if (!a || a == (AActor*)myPawn)
            continue;

        // If this field read is sometimes unsafe, you may need more pointer validation.
        if (a->physics != PHYS::Walking && a->physics != PHYS::Falling)
            continue;

        // Treat it as a pawn ONLY after your filters
        APawn* p = (APawn*)a;

        // Extra sanity on health reads
        if (p->health <= 0 || p->health > 10000)
            continue;

        pawns.push_back(p);
    }
    return pawns;
}

void Cheats::ScaleAActorsUp() {
    for (APawn* pawn : GetMovingPawns()) {
        if (!pawn) continue;
        pawn->x3DDrawScale *= 2.0f;
        pawn->y3DDrawScale *= 2.0f;
        pawn->z3DDrawScale *= 2.0f;
        pawn->drawSize *= 2.0f;
    }
}

void Cheats::ScaleAActorsDown() {
    for (APawn* pawn : GetMovingPawns()) {
        if (!pawn) continue;
        pawn->x3DDrawScale /= 2.0f;
        pawn->y3DDrawScale /= 2.0f;
        pawn->z3DDrawScale /= 2.0f;
        pawn->drawSize /= 2.0f;
        if (pawn->drawSize < 0.1f) pawn->drawSize = 0.1f;
    }
}

void Cheats::InstaKill() {
    for (APawn* pawn : GetMovingPawns()) {
        if (!pawn) continue;
        if (pawn->health > 5) pawn->health = 5;
    }
}

void Cheats::DrawMenu() {
    if (!bMenuOpen) return;
    ImGui::Begin("Killing Floor", &bMenuOpen, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Player Pointer: %p", myPawn);
    ImGui::Separator();
    ImGui::Text("InstaKill (F1): %s", bInstaKill ? "ON" : "OFF");
    ImGui::Text("GodMode (F2): %s", bGodMode ? "ON" : "OFF");
    ImGui::Text("F3: Double Size | F4: Half Size");
    ImGui::End();
}

void Cheats::Cleanup() {

    MH_DisableHook(endSceneAddress);
    MH_RemoveHook(endSceneAddress);
    MH_Uninitialize();

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    if (ImGui::GetCurrentContext()) ImGui::DestroyContext();
    if (oWndProc && gameWindow) {
        SetWindowLongPtr(gameWindow, GWLP_WNDPROC, (LONG_PTR)oWndProc);
    }
}

void Cheats::Start() {
    if (!CreateHook()) return;

    while (!bCanUnload) {
        if (!GetLocalPlayer()) {
            std::cout << "[-] Failed to fetch local player object, sleeping for 5 seconds" << std::endl;
            Sleep(5000);
            continue;
        }
        else if (myPawn->health <= 0) {
            std::cout << "[-] Localplayer is dead sleeping for 3 seconds" << std::endl;
            Sleep(3000);
            continue;
        }

        if (bInstaKill) InstaKill();

        if (bGodMode) {
            myPawn->health = 100;
        }
        Sleep(25);
    }
    Sleep(100);
    Cleanup();
}

void Cheats::ReleaseDevice() {
    if (pDummyDevice) { pDummyDevice->Release(); pDummyDevice = nullptr; }
    if (pD3D) { pD3D->Release(); pD3D = nullptr; }
    if (window) { DestroyWindow(window); window = nullptr; }
}

bool Cheats::CreateHook() {
    // 1.  Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        std::cout << "[-] Failed to initialize MinHook" << std::endl;
        return false;
    }

    // 2. Find the address
    if (!FetchEndSceneAddress()) {
        std::cout << "[-] Failed to fetch EndScene address" << std::endl;
        return false;
    }

    // 3. Create the hook
    if (MH_CreateHook(endSceneAddress, &hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene)) != MH_OK) {
        std::cout << "[-] Failed to create hook" << std::endl;
        return false;
    }

    // 4. Enable the hook
    if (MH_EnableHook(endSceneAddress) != MH_OK) {
        std::cout << "[-] Failed to enable hook" << std::endl;
        return false;
    }

    std::cout << "[+] Hook successfully applied!" << std::endl;
    return true;
}

bool Cheats::FetchEndSceneAddress() {
    window = CreateWindowExA(0, "STATIC", "DummyWindow", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, NULL, NULL);
    if (!window) return false;

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) { DestroyWindow(window); return false; }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = window;

    HRESULT res = pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pDummyDevice);
    if (FAILED(res) || !pDummyDevice) { ReleaseDevice(); return false; }

    void** vTable = *(void***)pDummyDevice;
    endSceneAddress = vTable[42];

    ReleaseDevice();
    return (endSceneAddress != nullptr);
}

Cheats cheats;