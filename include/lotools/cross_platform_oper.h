#pragma once

#include <filesystem>
#include <string>

#ifdef _WIN32
#    include <Windows.h>
#else
#    include <unistd.h>
#endif

#ifndef LOT_MAX_PATH_SIZE
#    define LOT_MAX_PATH_SIZE 512
#endif

namespace lot {

inline std::string get_executable_path()
{
    char buffer[LOT_MAX_PATH_SIZE];
#ifdef _WIN32
    ::GetModuleFileNameA(nullptr, buffer, LOT_MAX_PATH_SIZE);
#else
    ::readlink("/proc/self/exe", buffer, LOT_MAX_PATH_SIZE);
#endif
    std::filesystem::path exec_path = buffer;
    return exec_path.parent_path().string();
}

}; // namespace lot