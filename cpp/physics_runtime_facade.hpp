#ifndef PHYSICS_RUNTIME_FACADE_HPP
#define PHYSICS_RUNTIME_FACADE_HPP

#include <cstddef>
#include <unordered_map>
#include <vector>
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
    std::size_t runtime_error_count = 0;
};

struct TickSnapshot {
    std::size_t frame_index = 0;
    TickStats stats;
    ecs::BridgeValidationReport bridge;
};

enum class RuntimeErrorCode {
    BridgeMissingReverse,
    BridgeStaleEntity,
    BridgeNullBody,
    BridgeDuplicateBody,
    BridgeRefCountMismatch,
    PipelineMappingErrors
};

enum class RuntimeErrorSeverity {
    Warning,
    Error
};

inline const char* runtime_error_code_name(RuntimeErrorCode code) noexcept {
    switch (code) {
        case RuntimeErrorCode::BridgeMissingReverse: return "bridge_missing_reverse";
        case RuntimeErrorCode::BridgeStaleEntity: return "bridge_stale_entity";
        case RuntimeErrorCode::BridgeNullBody: return "bridge_null_body";
        case RuntimeErrorCode::BridgeDuplicateBody: return "bridge_duplicate_body";
        case RuntimeErrorCode::BridgeRefCountMismatch: return "bridge_ref_count_mismatch";
        case RuntimeErrorCode::PipelineMappingErrors: return "pipeline_mapping_errors";
    }
    return "unknown";
}

inline RuntimeErrorSeverity runtime_error_severity(RuntimeErrorCode code) noexcept {
    switch (code) {
        case RuntimeErrorCode::BridgeNullBody:
        case RuntimeErrorCode::BridgeDuplicateBody:
            return RuntimeErrorSeverity::Error;
        case RuntimeErrorCode::BridgeMissingReverse:
        case RuntimeErrorCode::BridgeStaleEntity:
        case RuntimeErrorCode::BridgeRefCountMismatch:
        case RuntimeErrorCode::PipelineMappingErrors:
            return RuntimeErrorSeverity::Warning;
    }
    return RuntimeErrorSeverity::Warning;
}

inline const char* runtime_error_severity_name(RuntimeErrorSeverity severity) noexcept {
    switch (severity) {
        case RuntimeErrorSeverity::Warning: return "warning";
        case RuntimeErrorSeverity::Error: return "error";
    }
    return "warning";
}

struct RuntimeError {
    RuntimeErrorCode code = RuntimeErrorCode::BridgeMissingReverse;
    RuntimeErrorSeverity severity = RuntimeErrorSeverity::Warning;
    std::size_t count = 0;
    std::size_t frame_index = 0;
};

enum class RuntimeEventKind {
    ContactCreated,
    BodySleep,
    BodyWake
};

struct RuntimeEvent {
    RuntimeEventKind kind = RuntimeEventKind::ContactCreated;
    ecs::Entity entity_a = ecs::kInvalidEntity;
    ecs::Entity entity_b = ecs::kInvalidEntity;
    int contact_index = -1;
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
        collect_errors(snapshot);
        snapshot.stats.runtime_error_count = last_errors_.size();
        collect_events(snapshot.stats.contact_count);
        last_snapshot_ = snapshot;
        return snapshot;
    }

    TickStats tick() noexcept {
        return tick_snapshot().stats;
    }

    const TickSnapshot& last_snapshot() const noexcept {
        return last_snapshot_;
    }

    const std::vector<RuntimeEvent>& last_events() const noexcept {
        return last_events_;
    }

    const std::vector<RuntimeError>& last_errors() const noexcept {
        return last_errors_;
    }

private:
    void push_error(RuntimeErrorCode code, std::size_t count, std::size_t frame_index) noexcept {
        if (count == 0) {
            return;
        }
        RuntimeError e;
        e.code = code;
        e.severity = runtime_error_severity(code);
        e.count = count;
        e.frame_index = frame_index;
        last_errors_.push_back(e);
    }

    void collect_errors(const TickSnapshot& snapshot) noexcept {
        last_errors_.clear();
        push_error(RuntimeErrorCode::BridgeMissingReverse, snapshot.bridge.missing_reverse, snapshot.frame_index);
        push_error(RuntimeErrorCode::BridgeStaleEntity, snapshot.bridge.stale_entities, snapshot.frame_index);
        push_error(RuntimeErrorCode::BridgeNullBody, snapshot.bridge.null_bodies, snapshot.frame_index);
        push_error(RuntimeErrorCode::BridgeDuplicateBody, snapshot.bridge.duplicate_bodies, snapshot.frame_index);
        if (snapshot.bridge.runtime_refs != snapshot.bridge.reverse_refs) {
            const std::size_t delta = (snapshot.bridge.runtime_refs > snapshot.bridge.reverse_refs)
                                          ? (snapshot.bridge.runtime_refs - snapshot.bridge.reverse_refs)
                                          : (snapshot.bridge.reverse_refs - snapshot.bridge.runtime_refs);
            push_error(RuntimeErrorCode::BridgeRefCountMismatch, delta, snapshot.frame_index);
        }
        push_error(RuntimeErrorCode::PipelineMappingErrors, snapshot.stats.mapping_errors, snapshot.frame_index);
    }

    void collect_events(int contact_count) noexcept {
        last_events_.clear();
        if (!engine_.valid()) {
            return;
        }

        for (auto it = last_sleep_state_.begin(); it != last_sleep_state_.end();) {
            if (!registry_.alive(it->first)) {
                it = last_sleep_state_.erase(it);
            } else {
                ++it;
            }
        }

        for (int i = 0; i < contact_count; i++) {
            const CollisionManifold* c = physics_engine_get_contact(engine_.get(), i);
            if (c == nullptr || c->bodyA == nullptr || c->bodyB == nullptr) {
                continue;
            }
            const ecs::Entity ea = registry_.entity_of_body(c->bodyA);
            const ecs::Entity eb = registry_.entity_of_body(c->bodyB);
            if (ea == ecs::kInvalidEntity || eb == ecs::kInvalidEntity) {
                continue;
            }
            RuntimeEvent e;
            e.kind = RuntimeEventKind::ContactCreated;
            e.entity_a = ea;
            e.entity_b = eb;
            e.contact_index = i;
            last_events_.push_back(e);
        }

        registry_.each_runtime([&](ecs::Entity entity, const ecs::RuntimeBodyRef& rr) {
            if (rr.body == nullptr) {
                return;
            }
            const bool current_sleeping = rr.body->sleeping != 0;
            auto it = last_sleep_state_.find(entity);
            if (it == last_sleep_state_.end()) {
                last_sleep_state_[entity] = current_sleeping;
                return;
            }
            if (it->second != current_sleeping) {
                RuntimeEvent e;
                e.kind = current_sleeping ? RuntimeEventKind::BodySleep : RuntimeEventKind::BodyWake;
                e.entity_a = entity;
                e.entity_b = ecs::kInvalidEntity;
                e.contact_index = -1;
                last_events_.push_back(e);
                it->second = current_sleeping;
            }
        });
    }

    Engine engine_;
    ecs::Registry registry_;
    ecs::Pipeline pipeline_;
    std::size_t frame_index_ = 0;
    TickSnapshot last_snapshot_;
    std::vector<RuntimeEvent> last_events_;
    std::vector<RuntimeError> last_errors_;
    std::unordered_map<ecs::Entity, bool> last_sleep_state_;
};

}  // namespace runtime
}  // namespace physics2d

#endif
