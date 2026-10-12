#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "augment/augment.hpp"

#pragma comment(lib, "ws2_32.lib")

// TODO: globals / state
// e.g. map of app socket -> original destination, mutex guarding it, etc.

void hook_connect(AugmentCtx* ctx, void* ud) {
    // TODO: pull args out of ctx, capture original dest,
    // rewrite to point at listener
}

static void install_hooks() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData); // ensure ws2_32 is initialized

    AugmentRegOpts opts = {};
    augment_register("connect", AUGMENT_PHASE_BEFORE, hook_connect, nullptr, &opts);

    // TODO: register WSAConnect / getaddrinfo hooks here once connect is working

    augment_install_all();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule); // skip unneeded notifications
            CreateThread(nullptr, 0,
                [](LPVOID) -> DWORD {
                    install_hooks();
                    return 0;
                },
                nullptr, 0, nullptr);
            break;
        case DLL_PROCESS_DETACH:
            // TODO: any cleanup
            break;
    }
    return TRUE;
}
