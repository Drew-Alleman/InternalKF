#include "pch.h"
#include "Cheats.h"
#include "ULevel.h"
#include "AActor.h" 
#include "APawn.h"
#include "Enums.h"
#include "APlayerController.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

WNDPROC oWndProc;
typedef HRESULT(STDMETHODCALLTYPE* EndScene_t)(IDirect3DDevice9*);
EndScene_t oEndScene = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // 1. Let ImGui see the message first
    if (cheats.bMenuOpen && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    // 2. Hotkeys
    if (uMsg == WM_KEYDOWN)
    {
        switch (wParam)
        {
        case VK_INSERT:
            cheats.bMenuOpen = !cheats.bMenuOpen;
            if (ImGui::GetCurrentContext())
            {
                ImGuiIO& io = ImGui::GetIO();
                io.MouseDrawCursor = cheats.bMenuOpen;

                // UNLOCK MOUSE: This prevents the game from snapping your cursor
                if (cheats.bMenuOpen) {
                    SetCursor(LoadCursor(NULL, IDC_ARROW));
                    ShowCursor(TRUE);
                }
            }
            return 0;
        case VK_END:
            cheats.bCanUnload = true;
            return 0;

        case VK_F1:
            cheats.bInstaKill = !cheats.bInstaKill;
            return 0;

        case VK_F2:
            cheats.bGodMode = !cheats.bGodMode;
            return 0;
        }
    }

    // 3. Block input to game if menu is open
    if (cheats.bMenuOpen) {
        ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
    }

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

    IDirect3DStateBlock9* pStateBlock = nullptr;
    if (pDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock) == D3D_OK) {
        pStateBlock->Capture();
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    cheats.RunCheats();
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

void Cheats::TargetEntity(APawn* target) {
    // 1. Validation: Ensure pointers are valid and target is alive
    if (!myPawn || !myController || !target || target->health <= 0) return;

    // 2. Position Deltas
    float dx = target->x - myPawn->x;
    float dy = target->y - myPawn->y;

    // Aim for the head: Target's base Z + height vs. My base Z + height
    float dz = (target->z + target->height) - (myPawn->z + myPawn->height);

    // 3. Distance Calculation
    float horizontalDist = sqrt(dx * dx + dy * dy);

    // Deadzone check: Prevent "aim jitter" when standing inside the target
    if (horizontalDist < 5.0f) return;

    // 4. Trigonometry (Radians)
    float yawRad = atan2(dx, dy);
    float pitchRad = atan2(dz, horizontalDist);

    // 5. Conversion to Unreal Units (Rotators)
    // 65536 / (2 * PI) = 10430.378
    const float UnrealModifier = 10430.378f;
    int newYaw = (int)(yawRad * UnrealModifier);
    int newPitch = (int)(pitchRad * UnrealModifier);

    if (newPitch > 16000) newPitch = 16000;
    else if (newPitch < -16000) newPitch = -16000;

    myController->Yaw = newYaw & 0xFFFF;
    myController->Pitch = newPitch;
}

APawn* Cheats::GetClosestEnemy(std::vector<APawn*> pawns) {
    APawn* closest = nullptr;
    float minDistanceSq = 100000000.0f; // Large value

    for (APawn* pawn : pawns) {

        if (pawn == nullptr) {
            continue;
        }

        // Skip teammates
        if (pawn->height == myPawn->height) {
            continue;
        }

        float dx = pawn->x - myPawn->x;
        float dy = pawn->y - myPawn->y;
        float dz = pawn->z - myPawn->z;

        float dist = (dx * dx) + (dy * dy) + (dz * dz);

        if (dist < minDistanceSq) {
            minDistanceSq = dist;
            closest = pawn;
        }
    }
    return closest;
}

bool Cheats::GetLocalPlayer() {
    if (!engineModule && !GetModules()) return false;

    __try {
        // 1. Get GEngine
        uintptr_t GEngine = *(uintptr_t*)(engineModule + 0x004C6934);
        if (!GEngine) return false;

        // 2. Get the Pawn (GEngine + 0x38)
        uintptr_t pawnAddr = *(uintptr_t*)(GEngine + 0x38);
        if (!pawnAddr) return false;
        this->myPawn = (APawn*)pawnAddr;

        // 3. Get the Controller directly using the offset from your screenshot
        // We read it as a uintptr_t first to verify it's not 0
        uintptr_t controllerAddr = *(uintptr_t*)(pawnAddr + 0x360);

        if (controllerAddr == 0) {
            this->myController = nullptr;
            return false;
        }

        this->myController = (AController*)controllerAddr;
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool IsValidActor(AActor* actor) {
    if (!actor) return false;

    __try {
        if (actor->height < 38.0f || actor->isBrush) return false;
        return (actor->physics == PHYS::Walking || actor->physics == PHYS::Falling);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

std::vector<APawn*> Cheats::GetMovingPawns() {
    std::vector<APawn*> pawns;

    if (!myPawn || !myPawn->Level) return pawns;

    ULevel* level = myPawn->Level;
    int count = level->currentEntities;

    // Use a local copy of the list to minimize race conditions
    for (int i = 0; i < count; i++) {
        AActor* a = level->EntityList[i];

        // Use our safe check function
        if (!IsValidActor(a) || a == (AActor*)myPawn) continue;

        APawn* p = (APawn*)a;

        // Final sanity check on pawn-specific data
        if (p->health <= 0 || p->health > 10000) continue;

        pawns.push_back(p);
    }
    return pawns;
}

void Cheats::ScaleAPawns(std::vector<APawn*> pawns) {
    for (APawn* pawn : pawns) {
        if (!pawn || pawn->x3DDrawScale == fZedScaleValue) continue;
        pawn->x3DDrawScale = fZedScaleValue;
        pawn->y3DDrawScale = fZedScaleValue;
        pawn->z3DDrawScale = fZedScaleValue;
        pawn->drawSize = fZedScaleValue;
    }
}


void Cheats::InstaKill(std::vector<APawn*> pawns) {
    for (APawn* pawn : pawns) {
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
    ImGui::SliderFloat("Enemy Size", &fZedScaleValue, 0.5f, 5.0f, "%.1f");

    if (ImGui::Button("Reset Scale")) {
        fZedScaleValue = 1.0f;
    }
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

void Cheats::RunCheats() {
    if (!GetLocalPlayer()) {
        return;
    }
    else if (myPawn->health <= 0) {
        return;
    }

    DrawCrosshair();

    std::vector<APawn*> pawns = GetMovingPawns();

    if (bInstaKill) InstaKill(pawns);

    if (bGodMode) {
        myPawn->health = 100;
    }

    ScaleAPawns(pawns);
    if (GetAsyncKeyState('Q') & 0x8000) {
        APawn* target = GetClosestEnemy(pawns);
        if (target) {
            TargetEntity(target);
        }
    }
}

void Cheats::Start() {
    if (!CreateHook()) {
        std::cout << "[-] Failed to hook EndScene. Thread exiting." << std::endl;
        return;
    }


    while (!bCanUnload) {
        Sleep(1000);
    }

    std::cout << "[+] Unloading sequence started..." << std::endl;
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

void Cheats::DrawCrosshair() {
    ImGuiIO& io = ImGui::GetIO();

    if (io.DisplaySize.x <= 0.0f || io.DisplaySize.y <= 0.0f)
        return;

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Find the center of the screen
    float centerX = io.DisplaySize.x / 2.0f;
    float centerY = io.DisplaySize.y / 2.0f;

    float length = 10.0f; // Length of the crosshair lines
    float thickness = 1.0f;
    float gap = 2.0f;      // Gap in the middle

    ImU32 color = IM_COL32(255, 0, 0, 255); // Red

    // Vertical Top
    drawList->AddLine(ImVec2(centerX, centerY - gap), ImVec2(centerX, centerY - gap - length), color, thickness);
    // Vertical Bottom
    drawList->AddLine(ImVec2(centerX, centerY + gap), ImVec2(centerX, centerY + gap + length), color, thickness);
    // Horizontal Left
    drawList->AddLine(ImVec2(centerX - gap, centerY), ImVec2(centerX - gap - length, centerY), color, thickness);
    // Horizontal Right
    drawList->AddLine(ImVec2(centerX + gap, centerY), ImVec2(centerX + gap + length, centerY), color, thickness);
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