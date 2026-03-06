#ifndef PHYSICS_SCRIPT_BRIDGE_HPP
#define PHYSICS_SCRIPT_BRIDGE_HPP

#include <string>
#include <unordered_map>
#include "physics_runtime_facade.hpp"

namespace physics2d {
namespace script {

enum class ColliderShape {
    Circle,
    Box
};

struct EntitySpawnSpec {
    std::string name;
    ecs::Transform transform{};
    ecs::RigidBodySpec body{};
    ColliderShape collider_shape = ColliderShape::Circle;
    float size_a = 0.5f;
    float size_b = 0.5f;
};

struct EntityState {
    ecs::Entity entity = ecs::kInvalidEntity;
    bool alive = false;
    bool runtime_ready = false;
    ecs::Transform transform{};
    Vec2 velocity = vec2(0.0f, 0.0f);
    float angular_velocity = 0.0f;
    BodyType body_type = BODY_DYNAMIC;
};

class ScriptBridge {
public:
    explicit ScriptBridge(runtime::RuntimeFacade& runtime) noexcept
        : runtime_(&runtime) {}

    ecs::Entity spawn_entity(const EntitySpawnSpec& spec) noexcept {
        if (runtime_ == nullptr) {
            return ecs::kInvalidEntity;
        }
        if (spec.name.empty() || names_.find(spec.name) != names_.end()) {
            return ecs::kInvalidEntity;
        }
        ecs::Registry& registry = runtime_->registry();
        const ecs::Entity entity = registry.create();
        registry.add_transform(entity, spec.transform);
        if (spec.collider_shape == ColliderShape::Circle) {
            registry.add_collider_circle(entity, spec.size_a);
        } else {
            registry.add_collider_box(entity, spec.size_a, spec.size_b);
        }
        registry.add_rigidbody_spec(entity, spec.body);
        names_[spec.name] = entity;
        return entity;
    }

    ecs::Entity entity_of(const std::string& name) const noexcept {
        const auto it = names_.find(name);
        if (it == names_.end()) {
            return ecs::kInvalidEntity;
        }
        return it->second;
    }

    bool has_entity(const std::string& name) const noexcept {
        if (runtime_ == nullptr) {
            return false;
        }
        const ecs::Entity entity = entity_of(name);
        return entity != ecs::kInvalidEntity && runtime_->registry().alive(entity);
    }

    bool set_transform(const std::string& name, const ecs::Transform& transform) noexcept {
        if (runtime_ == nullptr) {
            return false;
        }
        const ecs::Entity entity = entity_of(name);
        if (entity == ecs::kInvalidEntity || !runtime_->registry().alive(entity)) {
            return false;
        }
        runtime_->registry().add_transform(entity, transform);
        return true;
    }

    bool set_linear_velocity(const std::string& name, Vec2 velocity) noexcept {
        const ecs::RuntimeBodyRef* rr = runtime_body_of(name);
        if (rr == nullptr || rr->body == nullptr) {
            return false;
        }
        rr->body->velocity = velocity;
        rr->body->sleeping = 0;
        rr->body->sleep_timer = 0.0f;
        return true;
    }

    bool apply_impulse(const std::string& name, Vec2 impulse) noexcept {
        const ecs::RuntimeBodyRef* rr = runtime_body_of(name);
        if (rr == nullptr || rr->body == nullptr) {
            return false;
        }
        body_apply_impulse(rr->body, impulse);
        return true;
    }

    bool queue_destroy(const std::string& name) noexcept {
        if (runtime_ == nullptr) {
            return false;
        }
        const ecs::Entity entity = entity_of(name);
        if (entity == ecs::kInvalidEntity || !runtime_->registry().alive(entity)) {
            return false;
        }
        return runtime_->registry().queue_destroy(entity);
    }

    runtime::TickSnapshot tick() noexcept {
        runtime::TickSnapshot snapshot;
        if (runtime_ == nullptr) {
            return snapshot;
        }
        snapshot = runtime_->tick_snapshot();
        prune_dead_names();
        return snapshot;
    }

    EntityState read_state(const std::string& name) const noexcept {
        EntityState state;
        if (runtime_ == nullptr) {
            return state;
        }
        const ecs::Entity entity = entity_of(name);
        if (entity == ecs::kInvalidEntity) {
            return state;
        }
        state.entity = entity;
        state.alive = runtime_->registry().alive(entity);
        if (!state.alive) {
            return state;
        }
        if (const ecs::Transform* tr = runtime_->registry().transform(entity)) {
            state.transform = *tr;
        }
        if (const ecs::RuntimeBodyRef* rr = runtime_->registry().runtime_body(entity)) {
            if (rr->body != nullptr) {
                state.runtime_ready = true;
                state.transform.position = rr->body->position;
                state.transform.angle = rr->body->angle;
                state.velocity = rr->body->velocity;
                state.angular_velocity = rr->body->angular_velocity;
                state.body_type = rr->body->type;
            }
        }
        return state;
    }

private:
    const ecs::RuntimeBodyRef* runtime_body_of(const std::string& name) const noexcept {
        if (runtime_ == nullptr) {
            return nullptr;
        }
        const ecs::Entity entity = entity_of(name);
        if (entity == ecs::kInvalidEntity || !runtime_->registry().alive(entity)) {
            return nullptr;
        }
        return runtime_->registry().runtime_body(entity);
    }

    void prune_dead_names() noexcept {
        if (runtime_ == nullptr) {
            return;
        }
        for (auto it = names_.begin(); it != names_.end();) {
            if (!runtime_->registry().alive(it->second)) {
                it = names_.erase(it);
            } else {
                ++it;
            }
        }
    }

    runtime::RuntimeFacade* runtime_ = nullptr;
    std::unordered_map<std::string, ecs::Entity> names_;
};

}  // namespace script
}  // namespace physics2d

#endif
