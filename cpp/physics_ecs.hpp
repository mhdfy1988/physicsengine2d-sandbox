#ifndef PHYSICS_ECS_HPP
#define PHYSICS_ECS_HPP

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include "physics_body_raii.hpp"
#include "physics_raii.hpp"

namespace physics2d {
namespace ecs {

using Entity = std::uint32_t;
constexpr Entity kInvalidEntity = 0;

struct Transform {
    Vec2 position = vec2(0.0f, 0.0f);
    float angle = 0.0f;
};

enum class ColliderKind {
    Circle,
    Box
};

struct Collider {
    ColliderKind kind = ColliderKind::Circle;
    float size_a = 0.5f;
    float size_b = 0.5f;
};

struct RigidBodySpec {
    float mass = 1.0f;
    BodyType type = BODY_DYNAMIC;
};

struct RuntimeBodyRef {
    RigidBody* body = nullptr;
};

class Registry {
public:
    Entity create() noexcept {
        const Entity id = next_entity_++;
        alive_.insert(id);
        return id;
    }

    bool destroy(Entity e) noexcept {
        if (!alive(e)) {
            return false;
        }
        if (runtime_bodies_.find(e) != runtime_bodies_.end()) {
            return false;
        }

        erase_components(e);
        alive_.erase(e);
        destroy_queue_.erase(e);
        return true;
    }

    bool destroy(Entity e, EngineView engine) noexcept {
        if (!alive(e)) {
            return false;
        }

        auto it = runtime_bodies_.find(e);
        if (it != runtime_bodies_.end() && it->second.body != nullptr) {
            engine.remove_body(it->second.body);
            runtime_bodies_.erase(it);
        }

        erase_components(e);
        alive_.erase(e);
        destroy_queue_.erase(e);
        return true;
    }

    bool alive(Entity e) const noexcept {
        return alive_.find(e) != alive_.end();
    }

    void add_transform(Entity e, Transform t) noexcept {
        if (!alive(e)) {
            return;
        }
        transforms_[e] = t;
    }

    void add_collider_circle(Entity e, float radius) noexcept {
        if (!alive(e)) {
            return;
        }
        Collider c;
        c.kind = ColliderKind::Circle;
        c.size_a = radius;
        c.size_b = radius;
        colliders_[e] = c;
    }

    void add_collider_box(Entity e, float width, float height) noexcept {
        if (!alive(e)) {
            return;
        }
        Collider c;
        c.kind = ColliderKind::Box;
        c.size_a = width;
        c.size_b = height;
        colliders_[e] = c;
    }

    void add_rigidbody_spec(Entity e, RigidBodySpec spec) noexcept {
        if (!alive(e)) {
            return;
        }
        body_specs_[e] = spec;
    }

    const Transform* transform(Entity e) const noexcept {
        auto it = transforms_.find(e);
        if (it == transforms_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    const RuntimeBodyRef* runtime_body(Entity e) const noexcept {
        auto it = runtime_bodies_.find(e);
        if (it == runtime_bodies_.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool queue_destroy(Entity e) noexcept {
        if (!alive(e)) {
            return false;
        }
        destroy_queue_.insert(e);
        return true;
    }

    bool is_destroy_queued(Entity e) const noexcept {
        return destroy_queue_.find(e) != destroy_queue_.end();
    }

    std::size_t destroy_queued_count() const noexcept {
        return destroy_queue_.size();
    }

    std::size_t destroy_queued(EngineView engine) noexcept {
        std::size_t removed = 0;
        for (Entity e : destroy_queue_) {
            if (destroy(e, engine)) {
                removed++;
            }
        }
        destroy_queue_.clear();
        return removed;
    }

    template <typename Fn>
    void each_spawnable(Fn fn) const {
        for (Entity e : alive_) {
            if (runtime_bodies_.find(e) != runtime_bodies_.end()) {
                continue;
            }

            auto tr_it = transforms_.find(e);
            auto co_it = colliders_.find(e);
            auto sp_it = body_specs_.find(e);
            if (tr_it == transforms_.end() || co_it == colliders_.end() || sp_it == body_specs_.end()) {
                continue;
            }

            fn(e, tr_it->second, co_it->second, sp_it->second);
        }
    }

    template <typename Fn>
    void each_runtime(Fn fn) const {
        for (const auto& kv : runtime_bodies_) {
            fn(kv.first, kv.second);
        }
    }

    template <typename Fn>
    void each_runtime_with_transform(Fn fn) {
        for (auto& kv : runtime_bodies_) {
            auto tr_it = transforms_.find(kv.first);
            if (tr_it == transforms_.end()) {
                continue;
            }
            fn(kv.first, tr_it->second, kv.second);
        }
    }

    void spawn_rigid_bodies(EngineView engine) noexcept {
        each_spawnable([&](Entity e, const Transform& tr, const Collider& co, const RigidBodySpec& spec) {
            Body body;
            if (co.kind == ColliderKind::Circle) {
                body = Body::create_circle(tr.position.x, tr.position.y, spec.mass, co.size_a);
            } else {
                body = Body::create_box(tr.position.x, tr.position.y, spec.mass, co.size_a, co.size_b);
            }
            if (!body.valid()) {
                return;
            }

            body.set_type(spec.type);
            RigidBody* raw = body.get();
            engine.add_body(body.release());

            RuntimeBodyRef rr;
            rr.body = raw;
            runtime_bodies_[e] = rr;
        });
    }

    void sync_transforms_from_physics() noexcept {
        each_runtime_with_transform([](Entity, Transform& tr, const RuntimeBodyRef& rr) {
            if (rr.body == nullptr) {
                return;
            }

            tr.position = rr.body->position;
            tr.angle = rr.body->angle;
        });
    }

private:
    void erase_components(Entity e) noexcept {
        transforms_.erase(e);
        colliders_.erase(e);
        body_specs_.erase(e);
        runtime_bodies_.erase(e);
    }

    Entity next_entity_ = 1;
    std::unordered_set<Entity> alive_;
    std::unordered_map<Entity, Transform> transforms_;
    std::unordered_map<Entity, Collider> colliders_;
    std::unordered_map<Entity, RigidBodySpec> body_specs_;
    std::unordered_map<Entity, RuntimeBodyRef> runtime_bodies_;
    std::unordered_set<Entity> destroy_queue_;
};

class SpawnRigidBodySystem {
public:
    void run(Registry& registry, EngineView engine) const noexcept {
        registry.spawn_rigid_bodies(engine);
    }
};

class PhysicsStepSystem {
public:
    void run(EngineView engine) const noexcept {
        engine.step();
    }
};

class SyncTransformSystem {
public:
    void run(Registry& registry) const noexcept {
        registry.sync_transforms_from_physics();
    }
};

class CleanupSystem {
public:
    std::size_t run(Registry& registry, EngineView engine) const noexcept {
        return registry.destroy_queued(engine);
    }
};

struct PipelineStats {
    std::size_t cleaned_entities = 0;
};

class Pipeline {
public:
    void tick(Registry& registry, EngineView engine) noexcept {
        spawn_system_.run(registry, engine);
        step_system_.run(engine);
        sync_system_.run(registry);
        stats_.cleaned_entities = cleanup_system_.run(registry, engine);
    }

    const PipelineStats& stats() const noexcept {
        return stats_;
    }

private:
    SpawnRigidBodySystem spawn_system_;
    PhysicsStepSystem step_system_;
    SyncTransformSystem sync_system_;
    CleanupSystem cleanup_system_;
    PipelineStats stats_;
};

}  // namespace ecs
}  // namespace physics2d

#endif
