#pragma once
#include <string>
#include <filesystem>
#include <windows.h>

namespace supervisor {
// subprocess lifecycle

std::filesystem::path chooseProcess();
bool openSubprocess(std::string& process, PROCESS_INFORMATION& outProcInfo, bool suspended = false);

// interception

bool injectDll(const PROCESS_INFORMATION& info, const std::string& dllPath);

}
