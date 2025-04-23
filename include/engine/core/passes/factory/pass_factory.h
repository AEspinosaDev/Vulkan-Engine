
#ifndef PASS_FACTORY_H
#define PASS_FACTORY_H

#include <engine/common.h>
#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

using PassFactoryFunc = std::function<std::unique_ptr<Core::BasePass>(Graphics::Device*, void* /* config */)>;

class PassFactory
{
  private:
    std::unordered_map<std::string, PassFactoryFunc> m_factories;

  public:
    static PassFactory& instance() {
        static PassFactory reg;
        return reg;
    }

    void register_type(const std::string& type, PassFactoryFunc func) {
        m_factories[type] = std::move(func);
    }

    std::unique_ptr<Core::BasePass> create(const std::string& type, Graphics::Device* dev, void* config) {
        return m_factories.at(type)(dev, config);
    }
};

template <typename T, typename ConfigT>
std::unique_ptr<Core::BasePass> make_pass(Graphics::Device* device, void* config) {
    return std::make_unique<T>(device, *reinterpret_cast<ConfigT*>(config));
}

#define REGISTER_PASS(PassType, ConfigType, NameStr)                                                                   \
    static bool _##PassType##_registered = [] {                                                                        \
        PassFactory::instance().register_type(NameStr, &make_pass<PassType, ConfigType>);                              \
        return true;                                                                                                   \
    }()

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif