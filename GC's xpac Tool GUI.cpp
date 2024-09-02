#include "gcgui.hpp"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

int main() {

    bool done = false;
    gcGui gcg = gcGui("1.6.3", &done);

    MSG msg;
    while (!done) {
        GetMessageW(&msg, nullptr, 0, 0);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        gcg.Render();
        
    }

    exit(1);
}