#ifndef PHYSICS_RUNTIME_FACADE_HPP
#define PHYSICS_RUNTIME_FACADE_HPP

#include <cstddef>
#include "physics_ecs.hpp"
#include "physics_raii.hpp"

namespace physics2d {
namespace runtime {

struct TickStats {
    std::size_t spawned_entities = 0;
    std::size_t pre_synced_entities = 0;
    std::size_t respawned_entities = 0;
    std::size_t synced_entities = 0;
    std::size_t cleaned_entities = 0;
    std::size_t mapping_errors = 0;
    std::size_t runtime_body_refs = 0;
    std::size_t reverse_body_refs = 0;
    int body_count = 0;
    int contact_count = 0;
};

struct TickSnapshot {
    std::size_t frame_index = 0;
    TickStats stats;
    ecs::BridgeValidationReport bridge;
};

class RuntimeFacade {
public:
    RuntimeFacade() noexcept = default;

    static RuntimeFacade create() noexcept {
        RuntimeFacade rt;
        rt.engine_ = Engine::create();
        return rt;
    }

    bool valid() const noexcept {
        return engine_.valid();
    }

    Engine& engine() noexcept {
        return engine_;
    }

    const Engine& engine() const noexcept {
        return engine_;
    }

    ecs::Registry& registry() noexcept {
        return registry_;
    }

    const ecs::Registry& registry() const noexcept {
        return registry_;
    }

    ecs::Pipeline& pipeline() noexcept {
        return pipeline_;
    }

    const ecs::Pipeline& pipeline() const noexcept {
        return pipeline_;
    }

    TickSnapshot tick_snapshot() noexcept {
        TickSnapshot snapshot;
        if (!engine_.valid()) {
            return snapshot;
        }

        pipeline_.tick(registry_, engine_.view());
        snapshot.frame_index = ++frame_index_;
        snapshot.stats.spawned_entities = pipeline_.stats().spawned_entities;
        snapshot.stats.pre_synced_entities = pipeline_.stats().pre_synced_entities;
        snapshot.stats.respawned_entities = pipeline_.stats().respawned_entities;
        snapshot.stats.synced_entities = pipeline_.stats().synced_entities;
        snapshot.stats.cleaned_entities = pipeline_.stats().cleaned_entities;
        snapshot.stats.mapping_errors = pipeline_.stats().mapping_errors;
        snapshot.stats.body_count = engine_.body_count();
        snapshot.stats.contact_count = physics_engine_get_contact_count(engine_.get());
        snapshot.bridge = registry_.validate_bridge();
        snapshot.stats.runtime_body_refs = snapshot.bridge.runtime_refs;
        snapshot.stats.reverse_body_refs = snapshot.bridge.reverse_refs;
        last_snapshot_ = snapshot;
        return snapshot;
    }

    TickStats tick() noexcept {
        return tick_snapshot().stats;
    }

    const TickSnapshot& last_snapshot() const noexcept {
        return last_snapshot_;
    }

private:
    Engine engine_;
    ecs::Registry registry_;
    ecs::Pipeline pipeline_;
    std::size_t frame_index_ = 0;
    TickSnapshot last_snapshot_;
};

}  // namespace runtime
}  // namespace physics2d

#endif
