#include <windows.h>
#include "supervisor.hpp"

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
    // TODO: boot the local listener

    // 1: choose the process
    auto path = supervisor::chooseProcess();
    if (path.empty()) exit(-1);

    std::string pathStr = path.string();
    PROCESS_INFORMATION info{};

    // 2: open the process suspended
    bool opened = supervisor::openSubprocess(
        pathStr, 
        info, 
        true
    );
    if (!opened) exit(-1);

    // 3: hook the built DLL (intercept.cpp)
    auto dir = supervisor::getExeDirectory() / L"libintercept.dll";
    std::string fileStr = dir.string();
    bool injected = supervisor::injectDll(info, fileStr);
    if (!injected) exit(-1);

    // resume the subprocess
    ResumeThread(info.hThread);
    CloseHandle(info.hThread);
    CloseHandle(info.hProcess);
}
