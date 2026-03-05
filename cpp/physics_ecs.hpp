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

struct DirtySyncStats {
    std::size_t synced_entities = 0;
    std::size_t respawned_entities = 0;
};

struct BridgeValidationReport {
    std::size_t runtime_refs = 0;
    std::size_t reverse_refs = 0;
    std::size_t missing_reverse = 0;
    std::size_t stale_entities = 0;
    std::size_t null_bodies = 0;
    std::size_t duplicate_bodies = 0;

    bool ok() const noexcept {
        return missing_reverse == 0 &&
               stale_entities == 0 &&
               null_bodies == 0 &&
               duplicate_bodies == 0 &&
               runtime_refs == reverse_refs;
    }
};

class Registry {
public:
    enum DirtyMask : std::uint32_t {
        DirtyNone = 0u,
        DirtyTransform = 1u << 0,
        DirtyBodySpec = 1u << 1,
        DirtyCollider = 1u << 2
    };

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
            body_to_entity_.erase(it->second.body);
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
        mark_runtime_dirty(e, DirtyTransform);
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
        mark_runtime_dirty(e, DirtyCollider);
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
        mark_runtime_dirty(e, DirtyCollider);
    }

    void add_rigidbody_spec(Entity e, RigidBodySpec spec) noexcept {
        if (!alive(e)) {
            return;
        }
        body_specs_[e] = spec;
        mark_runtime_dirty(e, DirtyBodySpec);
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

    Entity entity_of_body(const RigidBody* body) const noexcept {
        auto it = body_to_entity_.find(body);
        if (it == body_to_entity_.end()) {
            return kInvalidEntity;
        }
        return it->second;
    }

    std::size_t runtime_body_count() const noexcept {
        return runtime_bodies_.size();
    }

    std::size_t dirty_entity_count() const noexcept {
        return runtime_dirty_.size();
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

    std::size_t spawn_rigid_bodies(EngineView engine) noexcept {
        std::size_t spawned = 0;
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
            body_to_entity_[raw] = e;
            runtime_dirty_.erase(e);
            spawned++;
        });
        return spawned;
    }

    DirtySyncStats pre_physics_sync_dirty(EngineView engine) noexcept {
        DirtySyncStats stats;
        std::unordered_set<Entity> pending;
        for (const auto& kv : runtime_dirty_) {
            pending.insert(kv.first);
        }
        for (Entity e : pending) {
            auto rt_it = runtime_bodies_.find(e);
            if (rt_it == runtime_bodies_.end() || rt_it->second.body == nullptr) {
                runtime_dirty_.erase(e);
                continue;
            }
            auto tr_it = transforms_.find(e);
            auto sp_it = body_specs_.find(e);
            auto co_it = colliders_.find(e);
            if (tr_it == transforms_.end() || sp_it == body_specs_.end() || co_it == colliders_.end()) {
                runtime_dirty_.erase(e);
                continue;
            }

            std::uint32_t mask = runtime_dirty_[e];
            RigidBody* current = rt_it->second.body;
            if ((mask & DirtyCollider) != 0u) {
                Body replacement;
                if (co_it->second.kind == ColliderKind::Circle) {
                    replacement = Body::create_circle(tr_it->second.position.x, tr_it->second.position.y, sp_it->second.mass, co_it->second.size_a);
                } else {
                    replacement = Body::create_box(tr_it->second.position.x, tr_it->second.position.y, sp_it->second.mass, co_it->second.size_a, co_it->second.size_b);
                }
                if (replacement.valid()) {
                    RigidBody* next = replacement.get();
                    next->angle = tr_it->second.angle;
                    next->velocity = current->velocity;
                    next->angular_velocity = current->angular_velocity;
                    body_set_type(next, sp_it->second.type);
                    body_to_entity_.erase(current);
                    engine.remove_body(current);
                    engine.add_body(replacement.release());
                    rt_it->second.body = next;
                    body_to_entity_[next] = e;
                    current = next;
                    stats.respawned_entities++;
                }
            }

            if ((mask & DirtyTransform) != 0u) {
                current->position = tr_it->second.position;
                current->angle = tr_it->second.angle;
                stats.synced_entities++;
            }
            if ((mask & DirtyBodySpec) != 0u) {
                body_set_type(current, sp_it->second.type);
                stats.synced_entities++;
            }
            runtime_dirty_.erase(e);
        }
        return stats;
    }

    std::size_t sync_transforms_from_physics() noexcept {
        std::size_t synced = 0;
        each_runtime_with_transform([](Entity, Transform& tr, const RuntimeBodyRef& rr) {
            if (rr.body == nullptr) {
                return;
            }

            tr.position = rr.body->position;
            tr.angle = rr.body->angle;
        });
        each_runtime_with_transform([&](Entity, Transform&, const RuntimeBodyRef& rr) {
            if (rr.body != nullptr) {
                synced++;
            }
        });
        return synced;
    }

    BridgeValidationReport validate_bridge() const noexcept {
        BridgeValidationReport report;
        std::unordered_set<const RigidBody*> dedup;
        report.runtime_refs = runtime_bodies_.size();
        report.reverse_refs = body_to_entity_.size();

        for (const auto& kv : runtime_bodies_) {
            const Entity e = kv.first;
            const RigidBody* body = kv.second.body;
            if (!alive(e)) {
                report.stale_entities++;
            }
            if (body == nullptr) {
                report.null_bodies++;
                continue;
            }
            if (!dedup.insert(body).second) {
                report.duplicate_bodies++;
            }
            auto rev = body_to_entity_.find(body);
            if (rev == body_to_entity_.end() || rev->second != e) {
                report.missing_reverse++;
            }
        }
        return report;
    }

private:
    void mark_runtime_dirty(Entity e, std::uint32_t bits) noexcept {
        if (runtime_bodies_.find(e) == runtime_bodies_.end()) {
            return;
        }
        runtime_dirty_[e] |= bits;
    }

    void erase_components(Entity e) noexcept {
        auto rt = runtime_bodies_.find(e);
        if (rt != runtime_bodies_.end() && rt->second.body != nullptr) {
            body_to_entity_.erase(rt->second.body);
        }
        transforms_.erase(e);
        colliders_.erase(e);
        body_specs_.erase(e);
        runtime_bodies_.erase(e);
        runtime_dirty_.erase(e);
    }

    Entity next_entity_ = 1;
    std::unordered_set<Entity> alive_;
    std::unordered_map<Entity, Transform> transforms_;
    std::unordered_map<Entity, Collider> colliders_;
    std::unordered_map<Entity, RigidBodySpec> body_specs_;
    std::unordered_map<Entity, RuntimeBodyRef> runtime_bodies_;
    std::unordered_map<const RigidBody*, Entity> body_to_entity_;
    std::unordered_map<Entity, std::uint32_t> runtime_dirty_;
    std::unordered_set<Entity> destroy_queue_;
};

class SpawnRigidBodySystem {
public:
    std::size_t run(Registry& registry, EngineView engine) const noexcept {
        return registry.spawn_rigid_bodies(engine);
    }
};

class PhysicsStepSystem {
public:
    void run(EngineView engine) const noexcept {
        engine.step();
    }
};

class PrePhysicsSyncSystem {
public:
    DirtySyncStats run(Registry& registry, EngineView engine) const noexcept {
        return registry.pre_physics_sync_dirty(engine);
    }
};

class SyncTransformSystem {
public:
    std::size_t run(Registry& registry) const noexcept {
        return registry.sync_transforms_from_physics();
    }
};

class CleanupSystem {
public:
    std::size_t run(Registry& registry, EngineView engine) const noexcept {
        return registry.destroy_queued(engine);
    }
};

struct PipelineStats {
    std::size_t spawned_entities = 0;
    std::size_t pre_synced_entities = 0;
    std::size_t respawned_entities = 0;
    std::size_t synced_entities = 0;
    std::size_t cleaned_entities = 0;
    std::size_t mapping_errors = 0;
};

struct PipelineConfig {
    bool run_spawn = true;
    bool run_pre_sync = true;
    bool run_step = true;
    bool run_sync = true;
    bool run_cleanup = true;
};

class Pipeline {
public:
    void set_config(const PipelineConfig& config) noexcept {
        config_ = config;
    }

    const PipelineConfig& config() const noexcept {
        return config_;
    }

    void tick(Registry& registry, EngineView engine) noexcept {
        stats_.spawned_entities = 0;
        if (config_.run_spawn) {
            stats_.spawned_entities = spawn_system_.run(registry, engine);
        }
        stats_.pre_synced_entities = 0;
        stats_.respawned_entities = 0;
        if (config_.run_pre_sync) {
            const DirtySyncStats ds = pre_sync_system_.run(registry, engine);
            stats_.pre_synced_entities = ds.synced_entities;
            stats_.respawned_entities = ds.respawned_entities;
        }
        if (config_.run_step) {
            step_system_.run(engine);
        }
        stats_.synced_entities = 0;
        stats_.mapping_errors = 0;
        if (config_.run_sync) {
            const BridgeValidationReport report = registry.validate_bridge();
            if (!report.ok()) {
                stats_.mapping_errors = report.missing_reverse +
                                        report.stale_entities +
                                        report.null_bodies +
                                        report.duplicate_bodies;
            }
            stats_.synced_entities = sync_system_.run(registry);
        }

        stats_.cleaned_entities = 0;
        if (config_.run_cleanup) {
            stats_.cleaned_entities = cleanup_system_.run(registry, engine);
        }
    }

    const PipelineStats& stats() const noexcept {
        return stats_;
    }

private:
    SpawnRigidBodySystem spawn_system_;
    PrePhysicsSyncSystem pre_sync_system_;
    PhysicsStepSystem step_system_;
    SyncTransformSystem sync_system_;
    CleanupSystem cleanup_system_;
    PipelineConfig config_;
    PipelineStats stats_;
};

}  // namespace ecs
}  // namespace physics2d

#endif
