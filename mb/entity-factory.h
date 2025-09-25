#pragma once
#include <entt/entt.hpp>
#include <filesystem>
#include <spdlog/spdlog.h>

class Entity_factory {
  public:
    Entity_factory(Entity_factory const &) = delete;
    Entity_factory(Entity_factory &&) = delete;
    Entity_factory &operator=(Entity_factory const &) = delete;
    Entity_factory &operator=(Entity_factory &&) = delete;
    // TODO(shelpam): Where to load these logic? We shouldn't hard code them
    // (maybe).
    explicit Entity_factory(entt::registry &registry) : registry_(registry) {}

    void register_prototype(std::string const &name, entt::entity id)
    {
        prototypes_[name] = id;
    }

    entt::entity make_entity(std::string const &prototype_name)
    {
        auto it = prototypes_.find(prototype_name);
        if (it == prototypes_.end()) {
            spdlog::error("unable to find prototype");
            throw std::runtime_error("check last error");
        }

        it->second;
    }

  private:
    entt::registry &registry_;
    std::unordered_map<std::string, entt::entity> prototypes_;
};
