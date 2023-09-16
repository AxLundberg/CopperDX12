#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

#include "EcsEntity.h"
#include "EcsSystem.h"

namespace CPR::ECS
{
    template <typename... Components>
    class ComponentCollector;

    template <typename... Args>
    struct are_present;

    class Ecs {
    public:
        Entity& createEntity();

        template<typename T, typename... Args>
        T& addComponent(Entity& entity, Args&&... args) {
            auto& componentsMap = components[typeid(T)];
            auto component = std::make_shared<T>(std::forward<Args>(args)...);
            componentsMap[entity.getID()] = component;
            return *component.get();
        }
        template<typename T>
        void removeComponent(Entity& entity) {
            auto it = components.find(typeid(T));
            if (it != components.end()) {
                it->second.erase(entity.getID());
                if (it->second.empty()) {
                    components.erase(it);
                }
            }
        }
        template<typename T>
        std::vector<Entity> getEntitiesWithComponent() {
            std::vector<Entity> entitiesWithComponent;

            auto it = components.find(typeid(T));

            if (it != components.end()) {
                auto& entityToComponentMap = it->second;

                for (const auto& [entityID, component] : entityToComponentMap) {
                    entitiesWithComponent.push_back(entities[entityID]);
                }
            }

            return entitiesWithComponent;
        }
        template<typename T>
        T* getComponent(Entity& entity) {
            auto it = components.find(typeid(T));
            if (it != components.end()) {
                auto& componentsMap = it->second;
                auto componentIt = componentsMap.find(entity.getID());
                if (componentIt != componentsMap.end()) {
                    return static_cast<T*>(componentIt->second.get());
                }
            }
            return nullptr;  // Component of type T not found for the entity
        };

        template<typename... Components>
        ComponentCollector<Components...> Collect() {
            return ComponentCollector<Components...>(*this);
        }

        template<typename... Args>
        bool HasComponents(int entityID) {
            return are_present<Args...>::in(components, entityID);
        }

        void removeEntity(Entity::ID entityID);

        Entity& getEntity(Entity::ID entityID);
        void removeEntity(Entity& entityID);
        void addSystem(std::unique_ptr<System> system);
        void updateSystems();

    public:
        std::unordered_map<Entity::ID, Entity> entities;
        // Using type_index as a key to store different component types
        std::unordered_map<std::type_index, std::unordered_map<Entity::ID, std::shared_ptr<void>>> components;
        std::vector<std::unique_ptr<System>> systems;
    };

    template <typename... Args>
    struct are_present;

    template <typename T, typename... Args>
    struct are_present<T, Args...> {
        static bool in(const std::unordered_map<std::type_index, std::unordered_map<Entity::ID, std::shared_ptr<void>>>& components, const Entity::ID& id) {
            auto it = components.find(typeid(T));
            if (it != components.end() && it->second.find(id) != it->second.end()) {
                return are_present<Args...>::in(components, id);
            }
            return false;
        }
    };

    template <>
    struct are_present<> {
        static bool in(const std::unordered_map<std::type_index, std::unordered_map<Entity::ID, std::shared_ptr<void>>>&, const Entity::ID&) {
            return true;
        }
    };

    template <typename First, typename... Rest>
    class ComponentCollector<First, Rest...> {
    private:
        Ecs& manager;
        std::vector<Entity> collectedEntities;

    public:
        ComponentCollector(Ecs& mgr) : manager(mgr) {
            for (const auto& [id, entity] : manager.entities) {
                if (are_present<First, Rest...>::in(manager.components, id)) {
                    collectedEntities.push_back(entity);
                }
            }
        }

        template<typename Func>
        void Do(Func&& func) {
            for (auto& e : collectedEntities) {
                func(e, *manager.getComponent<First>(e), *manager.getComponent<Rest>(e)...);
            }
        }
    };
}
//#include "Ecs.tpp"