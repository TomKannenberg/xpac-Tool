#include "gcgui.hpp"

void gcGui::FrontEnd() {

    ImColor MenuColor = ImVec4(menucolor[0], menucolor[1], menucolor[2], 1.0f);

    ImGui::SetNextWindowPos({ 0 , 0 }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(Gui::Size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground;

    ImGui::Begin(name.c_str(), nullptr, window_flags);

    auto draw = ImGui::GetWindowDrawList();

    auto pos = ImGui::GetWindowPos();
    auto size = ImGui::GetWindowSize();

    //draw->AddRectFilledMultiColor(pos, ImVec2(pos.x + 55, pos.y + 51), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.05f), ImColor(1.0f, 1.0f, 1.0f, 0.00f), ImColor(1.0f, 1.0f, 1.0f, 0.00f));
    draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + 51), ImColor(menucolor[0], menucolor[1], menucolor[2], 0.5f) / 2, 9.0f, ImDrawFlags_RoundCornersTop);

    ImGui::SetCursorPos(ImVec2(7, 5));
    constexpr ImVec2 logosize = ImVec2(40, 40);
    ImGui::Image(LogoI, logosize);
    draw->AddText(semibold, 17.0f, ImVec2(pos.x + 58, pos.y + 17), ImColor(192, 203, 229), name.c_str());

    cp = ImVec2(cursorpos.x, cursorpos.y);

    ImGui::SetCursorPos(ImVec2(Gui::Size.x - 7 - logosize.x, 5));
    inBox = isInsideBox(cp);
    if (!inBox) {
        ImGui::Image(ExitLogoI, logosize);
    } else {
        ImGui::Image(ExitLogoII, logosize);
    }

    UpdateWindowLocation();
    size_t xsize = 160;
    size_t ysize = 80;
    ImGui::SetCursorPos(ImVec2(Gui::Size.x / 2 - xsize / 1.75, ysize * 2 / 1.6));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5);

    ImGui::PushStyleColor(ImGuiCol_ButtonActive, purple3);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, purple2);
    ImGui::PushStyleColor(ImGuiCol_Button, purple);
    ImGui::PushStyleColor(ImGuiCol_Text, textcolor);
    if (ImGui::Button("Unpack", ImVec2(xsize, ysize))) {
        std::string folderPath2 = "0";
        folderPath2 = GetLocationXpac();
        if (folderPath2 == "0") {

        } else {
            unpackreturn = xpac->Unpack(folderPath2.c_str());

            rcd = 60;

            ImGui::OpenPopup("UnpackReturn");
        }
    }

    constexpr int extraoff = 12;
    ImGui::SetCursorPos(ImVec2(extraoff, ysize * 2 / 1.6));
    ImGui::Text("Extras:");
    ImGui::SetCursorPos(ImVec2(extraoff, ImGui::GetCursorPos().y));
    ImGui::Checkbox("Unpack Zif Assets", &xpac->upAssetInfo);
    ImGui::SetCursorPos(ImVec2(extraoff, ImGui::GetCursorPos().y));
    ImGui::Checkbox("Unpack c Files", &xpac->upCompressedFiles);
    ImGui::SetCursorPos(ImVec2(extraoff, ImGui::GetCursorPos().y));
    if (ImGui::Checkbox("Toggle Console", &xpac->debug)) {

        if (xpac->debug) {
            AllocConsole();
            freopen("CONOUT$", "w", stdout);
            hcon = GetConsoleWindow();
            SetForegroundWindow(hwnd);
        } else {
            FreeConsole();
            PostMessage(hcon, WM_CLOSE, 0, 0);
        }
    }

    const int xofs = Gui::Size.x * 0.7;
    ImGui::SetCursorPos(ImVec2(xofs + extraoff, ysize * 2 / 1.6));
    ImGui::Text("Repack Extras:");
    ImGui::SetCursorPos(ImVec2(xofs + extraoff, ImGui::GetCursorPos().y));
    ImGui::Checkbox("Repack Textures", &xpac->rpDDSFiles);

    ImGui::SetCursorPos(ImVec2(Gui::Size.x / 2 - xsize / 1.75, ysize * 4.5 / 1.6));

    if (ImGui::Button("Repack", ImVec2(xsize, ysize))) {
        std::string folderPath2 = "0";
        folderPath2 = GetLocationGpac();
        if (folderPath2 == "0") {
            std::cout << "still null";
        } else {
            unpackreturn = xpac->Repack(folderPath2.c_str());

            rcd = 60;

            ImGui::OpenPopup("UnpackReturn");
        }
    }
    //std::string text = std::to_string(cursorpos.x) + " " + std::to_string(cursorpos.y) + " " + std::to_string(wasDragged) + " " + std::to_string(mouseRecent);
    //ImGui::Text(text.c_str());

    if (ImGui::BeginPopupModal("UnpackReturn", nullptr, window_flags)) {

        ImGui::Text(unpackreturn.c_str());
        if (--rcd == 0) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();

    windowMove();

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

}

gcGui::gcGui(std::string version, bool* done) : done(done) {
    name += version;
    initHWND();
    Initialize();
    LPCSTR logo = LPCSTR(GCIMAGE1);
    cSprite(pDevice, logo);
    LogoI = newTexture;
    LPCSTR exitlogo = LPCSTR(GCIMAGE2);
    cSprite(pDevice, exitlogo);
    ExitLogoI = newTexture;
    LPCSTR exitlogo2 = LPCSTR(GCIMAGE3);
    cSprite(pDevice, exitlogo2);
    ExitLogoII = newTexture;

    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);

    folderPath = path;
    size_t lastSlash = folderPath.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        folderPath = folderPath.substr(0, lastSlash);
    }

    folderPath += "\\ExtractedXPAC";
    xpac = new XPAC(&folderPath);
    folderPath += "\\";

}

bool gcGui::isInsideBox(const ImVec2& cursorpos) {
    const static ImVec2 bl(456, 42);
    const static ImVec2 tr(491, 7);

    return (cursorpos.x >= bl.x && cursorpos.x <= tr.x &&
        cursorpos.y >= tr.y && cursorpos.y <= bl.y);
}

void gcGui::windowMove() {
    GetCursorPos(&cursorpos);
    ScreenToClient(hwnd, &cursorpos);
    bool isWindowDragged = ImGui::IsMouseDown(ImGuiMouseButton_Left);

    if (isWindowDragged) {

        if (!mouseRecent && !wasDragged) {
            if (inBox) {
                *done = true;
            }
        }

        if (!inBox && !mouseRecent && cursorpos.y < 50) {
            isWindowDragged = 1;
            wasDragged = 1;
            lastTime = GetTickCount();
        } else if (wasDragged || (GetTickCount() - lastTime) <= doubleClickTime) {
            isWindowDragged = 1;
            wasDragged = 1;
        } else {
            isWindowDragged = 0;
            mouseRecent = 1;
        }

    } else {
        wasDragged = 0;
        mouseRecent = 0;
    }

    if (isWindowDragged) {
        POINT currentPos;
        GetCursorPos(&currentPos);

        int dx = currentPos.x - lastMousePos.x;
        int dy = currentPos.y - lastMousePos.y;

        RECT windowRect;
        GetWindowRect(hwnd, &windowRect);

        SetWindowPos(
            hwnd,
            nullptr,
            windowRect.left + dx,
            windowRect.top + dy,
            0,
            0,
            SWP_NOSIZE | SWP_NOZORDER
        );

        lastMousePos = currentPos;
    } else {
        GetCursorPos(&lastMousePos);
    }
}

void gcGui::Render() {

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    FrontEnd();

    ImGui::Render();

    pDevice->SetRenderState(D3DRS_ZENABLE, false);
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
    pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

    pDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0, 0.0f, 0);

    if (pDevice->BeginScene() >= 0) {
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        pDevice->EndScene();
    }
    HRESULT result = pDevice->Present(nullptr, nullptr, nullptr, nullptr);

    if (result == D3DERR_DEVICELOST && pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
        ResetDevice();
    }

    std::this_thread::sleep_until(start + std::chrono::milliseconds(1));
}

void gcGui::UpdateWindowLocation() {
    GetWindowRect(hwnd, &rc);

    if (ImGui::GetWindowPos().x != 0 || ImGui::GetWindowPos().y != 0) {
        MoveWindow(hwnd, rc.left + ImGui::GetWindowPos().x, rc.top + ImGui::GetWindowPos().y, Gui::Size.x, Gui::Size.y, TRUE);
        ImGui::SetWindowPos(ImVec2(0.f, 0.f));
    }
}

gcGui::~gcGui() {
    Shutdown();
}

void gcGui::Initialize() {

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    ImGui::StyleColorsDark();

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);

    ImFontConfig font_config;
    font_config.PixelSnapH = false;
    font_config.OversampleH = 5;
    font_config.OversampleV = 5;
    font_config.RasterizerMultiply = 1.2f;

    static const ImWchar ranges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0xE000, 0xE226, // icons
        0
    };

    medium = io.Fonts->AddFontFromMemoryTTF((void*)InterMedium, sizeof(InterMedium), 15.0f, &font_config, ranges);
    semibold = io.Fonts->AddFontFromMemoryTTF((void*)InterSemiBold, sizeof(InterSemiBold), 17.0f, &font_config, ranges);

    IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) {
        return;
    }

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &pDevice))) {
        pD3D->Release();
        return;
    }

    pD3D->Release();

    CreateDeviceObjects();

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

void gcGui::Shutdown() {
    InvalidateDeviceObjects();
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX9_Shutdown();
    ImGui::DestroyContext();
}

void gcGui::CreateDeviceObjects() {
    if (!pDevice) { return; }
    ImGui_ImplDX9_Init(pDevice);
}

void gcGui::InvalidateDeviceObjects() {
    if (!pDevice) { return; }
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

constexpr unsigned char redge = 10;
void SetRoundedEdges(HWND hwnd, int width, int heights) {
    HRGN hRgn = CreateRoundRectRgn(0, 0, width, heights, redge, redge);
    SetWindowRgn(hwnd, hRgn, TRUE);
}

void ResetDevice() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = pDevice->Reset(&d3dpp);
    if (hr == D3DERR_INVALIDCALL) { IM_ASSERT(0); }
    ImGui_ImplDX9_CreateDeviceObjects();
}

void gcGui::RenderBlur() {
    struct ACCENTPOLICY {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary("user32.dll");
    if (hm) {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute) {
            ACCENTPOLICY policy = { 3, 0, 0, 0 };
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(hwnd, &data);
        }
        FreeLibrary(hm);
    }
}

void gcGui::initHWND() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandleA(NULL), NULL, NULL, NULL, NULL, name.c_str(), NULL };
    ::RegisterClassEx(&wc);
    hwnd = ::CreateWindowEx(0, wc.lpszClassName, name.c_str(), WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (Gui::Size.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (Gui::Size.y / 2), Gui::Size.x, Gui::Size.y, NULL, NULL, wc.hInstance, NULL);

    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & (WS_SYSMENU));
    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    RenderBlur();

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    GetWindowRect(hwnd, &rc);

    auto pref = DWMWCP_ROUND;
    HRESULT result = DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &pref, sizeof(int));
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) { return true; }
    int width = LOWORD(lParam);
    int height = HIWORD(lParam);
    RECT rc;

    switch (msg) {
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hWnd, &rect);

        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
        LOGBRUSH logBrush;
        GetObject(hBrush, sizeof(LOGBRUSH), &logBrush);
        logBrush.lbColor = RGB(0, 0, 0);
        logBrush.lbStyle = BS_SOLID;
        logBrush.lbHatch = 0;
        HBRUSH hTransparentBrush = CreateBrushIndirect(&logBrush);
        FillRect(hdc, &rect, hTransparentBrush);
        DeleteObject(hBrush);
        DeleteObject(hTransparentBrush);
        return 1;
    }

    case WM_SIZE:
        if (pDevice != NULL && wParam != SIZE_MINIMIZED) {
            d3dpp.BackBufferWidth = LOWORD(lParam);
            d3dpp.BackBufferHeight = HIWORD(lParam);
            ResetDevice();
        }
        SetRoundedEdges(hWnd, width, height);
        return 0;
        break;

    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) { return 0; }
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}