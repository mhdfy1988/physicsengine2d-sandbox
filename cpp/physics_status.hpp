#ifndef PHYSICS_STATUS_HPP
#define PHYSICS_STATUS_HPP

#include "../include/physics.h"

namespace physics2d {

struct Status {
    PhysicsErrorCode code = PHYSICS_ERROR_NONE;
    const char* message = "none";

    bool ok() const noexcept {
        return code == PHYSICS_ERROR_NONE;
    }
};

inline Status status_from_engine(const PhysicsEngine* engine) noexcept {
    Status s;
    s.code = physics_engine_get_last_error(engine);
    s.message = physics_error_code_string(s.code);
    return s;
}

}  // namespace physics2d

#endif
