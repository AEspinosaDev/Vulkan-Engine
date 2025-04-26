
#ifndef PASS_FACTORY_H
#define PASS_FACTORY_H

#include <engine/common.h>
#include <engine/core/passes/pass.h>

VULKAN_ENGINE_NAMESPACE_BEGIN

namespace Core {

class PassFactory
{
  public:
    static PassFactory& instance() {
        static PassFactory instance;
        return instance;
    }

    template <typename T, typename ConfigT> void register_pass(const std::string& name) {
        registry[name] = &make_pass<T, ConfigT>;
    }

    std::unique_ptr<BasePass> create(const std::string& name, Graphics::Device* dev, void* config) const {
        auto it = registry.find(name);
        if (it != registry.end())
        {
            return it->second(dev, config);
        }
        throw std::runtime_error("Pass type not registered: " + name);
    }

  private:
    using CreatorFunc = std::function<std::unique_ptr<BasePass>(Graphics::Device*, void*)>;
    std::unordered_map<std::string, CreatorFunc> registry;
    PassFactory() = default;
};

#define REGISTER_PASS(factory, Type, ConfigT) factory.register_pass<Type, ConfigT>(#Type)

#define REGISTER_PASS_ALIAS(factory, alias, Type, ConfigT) factory.register_pass<Type, ConfigT>(alias)

} // namespace Core

VULKAN_ENGINE_NAMESPACE_END

#endif