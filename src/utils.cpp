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

Vec3 Utils::get_tangent_gram_smidt(Vec3&      p1,
                                   Vec3&      p2,
                                   Vec3&      p3,
                                   Vec2& uv1,
                                   Vec2& uv2,
                                   Vec2& uv3,
                                   Vec3       normal) {

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


VULKAN_ENGINE_NAMESPACE_END