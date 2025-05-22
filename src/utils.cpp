#include <engine/utils.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

std::string Utils::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last  = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}
std::string Utils::read_file(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open #include script: " + filePath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

Vec3 Utils::get_tangent_gram_smidt(Vec3& p1, Vec3& p2, Vec3& p3, Vec2& uv1, Vec2& uv2, Vec2& uv3, Vec3 normal) {

    Vec3      edge1    = p2 - p1;
    Vec3      edge2    = p3 - p1;
    glm::vec2 deltaUV1 = uv2 - uv1;
    glm::vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
    Vec3  tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    // Gram-Schmidt orthogonalization
    return glm::normalize(tangent - normal * glm::dot(normal, tangent));

    // return glm::normalize(tangent);
}

size_t Utils::get_channel_count(ColorFormatType colorFormatType) {
    switch (colorFormatType)
    {
    case ColorFormatType::SR_8:
    case ColorFormatType::SR_16F:
    case ColorFormatType::SR_32F:
    case ColorFormatType::R_32_UINT:
    case ColorFormatType::R_8U:
        return 1;

    case ColorFormatType::SRG_8:
    case ColorFormatType::SRG_16F:
    case ColorFormatType::SRG_32F:
    case ColorFormatType::RG_8U:
        return 2;

    case ColorFormatType::SRGB_8:
    case ColorFormatType::SRGB_32F:
    case ColorFormatType::RGB_8U:
        return 3;

    case ColorFormatType::SRGBA_8:
    case ColorFormatType::SBGRA_8:
    case ColorFormatType::SRGBA_16F:
    case ColorFormatType::SRGBA_32F:
    case ColorFormatType::RGBA_8U:
    case ColorFormatType::RGB10A2:
        return 4;

    case ColorFormatType::DEPTH_16F:
    case ColorFormatType::DEPTH_32F:
        return 1; // Depth formats are single-channel

    default:
        throw std::invalid_argument("VKEngine error: Unknown ColorFormatType in get_channel_count");
    }
}

bool Utils::is_hdr_format(ColorFormatType colorFormatType) {
    switch (colorFormatType)
    {
    // 16-bit float formats
    case ColorFormatType::SR_16F:
    case ColorFormatType::SRG_16F:
    case ColorFormatType::SRGBA_16F:

    // 32-bit float formats
    case ColorFormatType::SR_32F:
    case ColorFormatType::SRG_32F:
    case ColorFormatType::SRGB_32F:
    case ColorFormatType::SRGBA_32F:

    // Depth 32F could be considered HDR in context of linear depth buffers
    case ColorFormatType::DEPTH_32F:
        return true;

    default:
        return false;
    }
}

size_t Utils::get_pixel_size_in_bytes(ColorFormatType format) {
    switch (format)
    {
    case ColorFormatType::SR_8:
    case ColorFormatType::R_8U:
        return 1;

    case ColorFormatType::SRG_8:
    case ColorFormatType::RG_8U:
        return 2;

    case ColorFormatType::SRGB_8:
    case ColorFormatType::RGB_8U:
        return 3;

    case ColorFormatType::SRGBA_8:
    case ColorFormatType::SBGRA_8:
    case ColorFormatType::RGBA_8U:
        return 4;

    case ColorFormatType::SR_16F:
        return 2;
    case ColorFormatType::SR_32F:
    case ColorFormatType::R_32_UINT:
        return 4;

    case ColorFormatType::SRG_16F:
        return 4;
    case ColorFormatType::SRG_32F:
        return 8;

    case ColorFormatType::SRGBA_16F:
        return 8;
    case ColorFormatType::SRGBA_32F:
        return 16;

    case ColorFormatType::SRGB_32F:
        return 12;

    case ColorFormatType::DEPTH_16F:
        return 2;
    case ColorFormatType::DEPTH_32F:
        return 4;

    case ColorFormatType::RGB10A2:
        return 4;

    default:
        throw std::invalid_argument("Unknown format in get_pixel_size");
    }
}

ImageAspect Utils::get_aspect(ColorFormatType format) {
    switch (format)
    {
    // Color formats
    case ColorFormatType::SR_8:
    case ColorFormatType::R_8U:
    case ColorFormatType::SRG_8:
    case ColorFormatType::RG_8U:
    case ColorFormatType::SRGB_8:
    case ColorFormatType::RGB_8U:
    case ColorFormatType::SRGBA_8:
    case ColorFormatType::SBGRA_8:
    case ColorFormatType::RGBA_8U:
    case ColorFormatType::SR_16F:
    case ColorFormatType::SR_32F:
    case ColorFormatType::R_32_UINT:
    case ColorFormatType::SRG_16F:
    case ColorFormatType::SRG_32F:
    case ColorFormatType::SRGBA_16F:
    case ColorFormatType::SRGBA_32F:
    case ColorFormatType::SRGB_32F:
    case ColorFormatType::RGB10A2:
        return ASPECT_COLOR;

    // Depth-only formats
    case ColorFormatType::DEPTH_16F:
    case ColorFormatType::DEPTH_32F:
        return ASPECT_DEPTH;

    default:
        throw std::invalid_argument("Unknown format in get_aspect_flags");
    }
}
VULKAN_ENGINE_NAMESPACE_END