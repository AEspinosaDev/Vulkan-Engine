#include <engine/common.h>

std::filesystem::path get_executable_dir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
#elif __linux__
    char    buffer[1024];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    return std::filesystem::path(buffer).parent_path();
#endif
}


const std::filesystem::path VKFW::Paths::RESOURCES_PATH = DISTRIBUTION_MODE ? get_executable_dir().parent_path() / "resources" : ENGINE_RESOURCES_PATH;