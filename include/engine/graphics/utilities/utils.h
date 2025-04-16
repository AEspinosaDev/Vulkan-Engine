/*
    This file is part of Vulkan-Engine, a simple to use Vulkan based 3D library

    MIT License

    Copyright (c) 2023 Antonio Espinosa Garcia

*/
#ifndef UTILS_H
#define UTILS_H

#include <chrono>
#include <deque>
#include <engine/common.h>
#include <engine/graphics/utilities/initializers.h>
#include <functional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics {

namespace Utils {




struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {
        deletors.push_back(function);
    }

    void flush() {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++)
        {
            (*it)(); // call functors
        }

        deletors.clear();
    }
};
class ManualTimer
{
    std::chrono::high_resolution_clock::time_point t0;
    double                                         timestamp{0.0};

  public:
    void start() {
        t0 = std::chrono::high_resolution_clock::now();
    }
    void stop() {
        timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count() * 1000.0;
    }
    const double& get() {
        return timestamp;
    }
};

struct memory_buffer : public std::streambuf {
    char*  p_start{nullptr};
    char*  p_end{nullptr};
    size_t size;

    memory_buffer(char const* first_elem, size_t size)
        : p_start(const_cast<char*>(first_elem))
        , p_end(p_start + size)
        , size(size) {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        if (dir == std::ios_base::cur)
            gbump(static_cast<int>(off));
        else
            setg(p_start, (dir == std::ios_base::beg ? p_start : p_end) + off, p_end);
        return gptr() - p_start;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream : virtual memory_buffer, public std::istream {
    memory_stream(char const* first_elem, size_t size)
        : memory_buffer(first_elem, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

// Function to trim leading and trailing whitespace
std::string trim(const std::string& str);

std::string read_file(const std::string& filePath);

inline std::vector<uint8_t> read_file_binary(const std::string& pathToFile) {
    std::ifstream        file(pathToFile, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t sizeBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (file.read((char*)fileBufferBytes.data(), sizeBytes))
            return fileBufferBytes;
    } else
        throw std::runtime_error("could not open binary ifstream to path " + pathToFile);
    return fileBufferBytes;
}


Vec3 get_tangent_gram_smidt(Vec3& p1, Vec3& p2, Vec3& p3, glm::vec2& uv1, glm::vec2& uv2, glm::vec2& uv3, Vec3 normal);

}; // namespace Utils
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif