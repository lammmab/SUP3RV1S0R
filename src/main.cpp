#include <windows.h>
#include "supervisor.hpp"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    // TODO: boot the local listener

    auto path = supervisor::chooseProcess();
    if (path.empty()) exit(-1);
    std::string pathStr = path.string();
    PROCESS_INFORMATION info{};
    bool success = supervisor::openSubprocess(
        pathStr, 
        info, 
        true
    );
    if (!success) exit(-1);

    // TODO: actually hook the DLL (supervisor::injectDll)

    // resume the subprocess
    ResumeThread(info.hThread);
    CloseHandle(info.hThread);
    CloseHandle(info.hProcess);
}
