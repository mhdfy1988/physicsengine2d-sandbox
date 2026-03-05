#ifndef PHYSICS_RUNTIME_FACADE_HPP
#define PHYSICS_RUNTIME_FACADE_HPP

#include <cstddef>
#include "physics_ecs.hpp"
#include "physics_raii.hpp"

namespace physics2d {
namespace runtime {

struct TickStats {
    std::size_t cleaned_entities = 0;
    int body_count = 0;
    int contact_count = 0;
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

    TickStats tick() noexcept {
        TickStats s;
        if (!engine_.valid()) {
            return s;
        }

        pipeline_.tick(registry_, engine_.view());
        s.cleaned_entities = pipeline_.stats().cleaned_entities;
        s.body_count = engine_.body_count();
        s.contact_count = physics_engine_get_contact_count(engine_.get());
        return s;
    }

private:
    Engine engine_;
    ecs::Registry registry_;
    ecs::Pipeline pipeline_;
};

}  // namespace runtime
}  // namespace physics2d

#endif
