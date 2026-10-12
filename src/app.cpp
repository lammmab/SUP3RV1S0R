/**
 * @file app.cpp
 *
 * Subprocess flow:
 *    1. Spin it up suspended (CREATE_SUSPENDED)
 *    2. Inject the DLL (built from intercept.cpp)
 *    3. Resume the process.
 */

#include "supervisor.hpp"
#include <shobjidl.h>

namespace supervisor {

bool openSubprocess(
    std::string& process,
    PROCESS_INFORMATION& outProcInfo,
    bool suspended
) {
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = {};

    std::string cmdLine = process;

    DWORD flags = suspended ? CREATE_SUSPENDED : 0;

    BOOL ok = CreateProcessA(
        nullptr,        // no separate application name, use cmdLine
        cmdLine.data(), // writable buffer
        nullptr,        // default process security attributes
        nullptr,        // default thread security attributes
        FALSE,          // don't inherit handles
        flags,
        nullptr,        // use parent's environment
        nullptr,        // use parent's current directory
        &si,
        &pi
    );

    if (!ok) {
        return false;
    }

    outProcInfo = pi;
    return true;
}

std::filesystem::path chooseProcess()
{
    std::filesystem::path result;

    if (FAILED(CoInitializeEx(
        nullptr,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)))
        return result;

    IFileOpenDialog* dialog = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_FileOpenDialog,
        nullptr,
        CLSCTX_ALL,
        IID_PPV_ARGS(&dialog)
    );

    if (SUCCEEDED(hr))
    {
        COMDLG_FILTERSPEC filters[] = {
            { L"Executable Files", L"*.exe" },
            { L"All Files", L"*.*" }
        };

        dialog->SetFileTypes(2, filters);
        dialog->SetTitle(L"Choose a process executable");

        hr = dialog->Show(nullptr);

        if (SUCCEEDED(hr))
        {
            IShellItem* item = nullptr;

            if (SUCCEEDED(dialog->GetResult(&item)))
            {
                PWSTR path = nullptr;

                if (SUCCEEDED(item->GetDisplayName(
                    SIGDN_FILESYSPATH, &path)))
                {
                    result = path;
                    CoTaskMemFree(path);
                }

                item->Release();
            }
        }

        dialog->Release();
    }

    CoUninitialize();
    return result;
}

bool injectDll(const PROCESS_INFORMATION& info, const std::string& dllPath) {
    // allocate mem in the target process for the DLL path string
    SIZE_T pathSize = dllPath.size() + 1; // +1 for null terminator
    LPVOID remotePathAddr = VirtualAllocEx(
        info.hProcess,
        nullptr,
        pathSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
    if (!remotePathAddr) return false;

    // write the DLL path into that remote memory
    if (!WriteProcessMemory(
            info.hProcess,
            remotePathAddr,
            dllPath.c_str(),
            pathSize,
            nullptr)) {
        VirtualFreeEx(info.hProcess, remotePathAddr, 0, MEM_RELEASE);
        return false;
    }

    // resolve LoadLibraryA's address
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32) return false;

    LPTHREAD_START_ROUTINE loadLibraryAddr =
        (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryA");
    if (!loadLibraryAddr) return false;

    // create a remote thread in the target process that calls LoadLibraryA(remotePathAddr)
    HANDLE hRemoteThread = CreateRemoteThread(
        info.hProcess,
        nullptr,
        0,
        loadLibraryAddr,
        remotePathAddr,
        0,
        nullptr
    );
    if (!hRemoteThread) {
        VirtualFreeEx(info.hProcess, remotePathAddr, 0, MEM_RELEASE);
        return false;
    }

    // wait for LoadLibrary to finish before resuming the suspended
    WaitForSingleObject(hRemoteThread, INFINITE);

    CloseHandle(hRemoteThread);
    VirtualFreeEx(info.hProcess, remotePathAddr, 0, MEM_RELEASE);
    return true;
}

}
