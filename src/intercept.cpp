#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "augment/augment.hpp"
#include "net.hpp"

#pragma comment(lib, "ws2_32.lib")

// TODO: globals / state
// e.g. map of app socket -> original destination, mutex guarding it, etc.

// https://learn.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-connect
void hook_connect(AugmentCtx* ctx, void* ud) {
    // get the address info from the real connect call
    sockaddr_in* addr = (sockaddr_in*)ctx->args[1];

    // grab the ip:port from the call
    // TODO: save the og
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr->sin_addr, ipStr, sizeof(ipStr));
    int port = ntohs(addr->sin_port);

    // rewrite it to point at our local listener instead
    inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr);
    addr->sin_port = htons(net::PORT);
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
