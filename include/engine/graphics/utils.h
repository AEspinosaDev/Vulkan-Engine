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
#include <engine/graphics/initializers.h>
#include <functional>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Graphics
{

namespace utils
{

struct UploadContext
{
    VkFence uploadFence;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    void init(VkDevice &device, VkPhysicalDevice &gpu, VkSurfaceKHR surface);
    void cleanup(VkDevice &device);
    void immediate_submit(VkDevice &device, VkQueue &gfxQueue, std::function<void(VkCommandBuffer cmd)> &&function);
};

struct DeletionQueue
{
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()> &&function)
    {
        deletors.push_back(function);
    }

    void flush()
    {
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
    double timestamp{0.0};

  public:
    void start()
    {
        t0 = std::chrono::high_resolution_clock::now();
    }
    void stop()
    {
        timestamp = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t0).count() * 1000.0;
    }
    const double &get()
    {
        return timestamp;
    }
};

/*
Canonical vertex definition. Most meshes will use some or all of these attributes.
*/
struct Vertex
{
    Vec3 pos;
    Vec3 normal;
    Vec3 tangent;
    Vec2 texCoord;
    Vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions(
        bool position = true, bool normal = true, bool tangent = true, bool texCoord = true, bool color = true)
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
        if (position)
        {
            VkVertexInputAttributeDescription posAtt{};
            posAtt.binding = 0;
            posAtt.location = 0;
            posAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            posAtt.offset = offsetof(Vertex, pos);
            attributeDescriptions.push_back(posAtt);
        }
        if (normal)
        {
            VkVertexInputAttributeDescription normalAtt{};
            normalAtt.binding = 0;
            normalAtt.location = 1;
            normalAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            normalAtt.offset = offsetof(Vertex, normal);
            attributeDescriptions.push_back(normalAtt);
        }
        if (texCoord)
        {
            VkVertexInputAttributeDescription texCoordAtt{};
            texCoordAtt.binding = 0;
            texCoordAtt.location = 2;
            texCoordAtt.format = VK_FORMAT_R32G32_SFLOAT;
            texCoordAtt.offset = offsetof(Vertex, texCoord);
            attributeDescriptions.push_back(texCoordAtt);
        }
        if (tangent)
        {
            VkVertexInputAttributeDescription tangentAtt{};
            tangentAtt.binding = 0;
            tangentAtt.location = 3;
            tangentAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            tangentAtt.offset = offsetof(Vertex, tangent);
            attributeDescriptions.push_back(tangentAtt);
        }
        if (color)
        {
            VkVertexInputAttributeDescription colorAtt{};
            colorAtt.binding = 0;
            colorAtt.location = 4;
            colorAtt.format = VK_FORMAT_R32G32B32_SFLOAT;
            colorAtt.offset = offsetof(Vertex, color);
            attributeDescriptions.push_back(colorAtt);
        }

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const
    {
        return pos == other.pos && normal == other.normal && tangent == other.tangent && texCoord == other.texCoord &&
               color == other.color;
    }

    bool operator!=(const Vertex &other) const
    {
        return !(*this == other);
    }
};

struct memory_buffer : public std::streambuf
{
    char *p_start{nullptr};
    char *p_end{nullptr};
    size_t size;

    memory_buffer(char const *first_elem, size_t size)
        : p_start(const_cast<char *>(first_elem)), p_end(p_start + size), size(size)
    {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override
    {
        if (dir == std::ios_base::cur)
            gbump(static_cast<int>(off));
        else
            setg(p_start, (dir == std::ios_base::beg ? p_start : p_end) + off, p_end);
        return gptr() - p_start;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override
    {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream : virtual memory_buffer, public std::istream
{
    memory_stream(char const *first_elem, size_t size)
        : memory_buffer(first_elem, size), std::istream(static_cast<std::streambuf *>(this))
    {
    }
};

// Function to trim leading and trailing whitespace
std::string trim(const std::string &str);

std::string read_file(const std::string &filePath);

inline std::vector<uint8_t> read_file_binary(const std::string &pathToFile)
{
    std::ifstream file(pathToFile, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        size_t sizeBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (file.read((char *)fileBufferBytes.data(), sizeBytes))
            return fileBufferBytes;
    }
    else
        throw std::runtime_error("could not open binary ifstream to path " + pathToFile);
    return fileBufferBytes;
}

struct EventDispatcher
{

    std::vector<std::function<void()>> functions;

    void push_function(std::function<void()> &&function)
    {
        functions.push_back(function);
    }

    void dispatch()
    {
        // reverse iterate the deletion queue to execute all the functions
        for (auto it = functions.rbegin(); it != functions.rend(); it++)
        {
            (*it)(); // call functors
        }

        functions.clear();
    }
};

VkPhysicalDeviceProperties get_gpu_properties(VkPhysicalDevice &gpu);

VkPhysicalDeviceFeatures get_gpu_features(VkPhysicalDevice &gpu);
inline bool is_instance_extension_supported(const char *extensionName)
{
    uint32_t extensionCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    for (const auto &extension : extensions)
    {
        if (strcmp(extension.extensionName, extensionName) == 0)
        {
            return true;
        }
    }

    return false;
}

// Check if device extension is supported
inline bool is_device_extension_supported(VkPhysicalDevice &physicalDevice, const char *extensionName)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());

    for (const auto &extension : extensions)
    {
        if (strcmp(extension.extensionName, extensionName) == 0)
        {
            return true;
        }
    }

    return false;
}

size_t pad_uniform_buffer_size(size_t originalSize, VkPhysicalDevice &gpu);

uint32_t find_memory_type(VkPhysicalDevice &gpu, uint32_t typeFilter, VkMemoryPropertyFlags properties);

bool check_validation_layer_suport(std::vector<const char *> validationLayers);

void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                           void *pUserData)
{

    std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                          const VkAllocationCallbacks *pAllocator,
                                          VkDebugUtilsMessengerEXT *pDebugMessenger);

void destroy_debug_utils_messenger_EXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                       const VkAllocationCallbacks *pAllocator);

void log_available_extensions(std::vector<VkExtensionProperties> ext);

void log_available_gpus(std::multimap<int, VkPhysicalDevice> candidates);

Vec3 get_tangent_gram_smidt(Vec3 &p1, Vec3 &p2, Vec3 &p3, glm::vec2 &uv1, glm::vec2 &uv2, glm::vec2 &uv3, Vec3 normal);
void compute_tangents_gram_smidt(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

template <typename T, typename... Rest> void hash_combine(std::size_t &seed, const T &v, const Rest &...rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}

}; // namespace utils
} // namespace Graphics

VULKAN_ENGINE_NAMESPACE_END

#endif