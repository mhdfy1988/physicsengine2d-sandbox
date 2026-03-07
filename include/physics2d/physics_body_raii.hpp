#ifndef PHYSICS_BODY_RAII_HPP
#define PHYSICS_BODY_RAII_HPP

#include <memory>
#include "physics_core/body.hpp"
#include "physics_core/shape.hpp"

namespace physics2d {

struct ShapeDeleter {
    void operator()(Shape* shape) const noexcept {
        if (shape != nullptr) {
            shape_free(shape);
        }
    }
};

class ShapeHandle {
public:
    using Handle = std::unique_ptr<Shape, ShapeDeleter>;

    ShapeHandle() noexcept = default;
    explicit ShapeHandle(Shape* raw) noexcept : handle_(raw) {}

    static ShapeHandle circle(float radius) noexcept {
        return ShapeHandle(shape_create_circle(radius));
    }

    static ShapeHandle box(float width, float height) noexcept {
        return ShapeHandle(shape_create_box(width, height));
    }

    bool valid() const noexcept {
        return handle_ != nullptr;
    }

    Shape* get() const noexcept {
        return handle_.get();
    }

    Shape* release() noexcept {
        return handle_.release();
    }

    void reset(Shape* raw = nullptr) noexcept {
        handle_.reset(raw);
    }

private:
    Handle handle_;
};

struct BodyDeleter {
    void operator()(RigidBody* body) const noexcept {
        if (body != nullptr) {
            body_free(body);
        }
    }
};

class Body {
public:
    using Handle = std::unique_ptr<RigidBody, BodyDeleter>;

    Body() noexcept = default;
    explicit Body(RigidBody* raw) noexcept : handle_(raw) {}

    static Body create(float x, float y, float mass, ShapeHandle&& shape) noexcept {
        if (!shape.valid()) {
            return Body();
        }

        RigidBody* raw = body_create(x, y, mass, shape.release());
        return Body(raw);
    }

    static Body create_circle(float x, float y, float mass, float radius) noexcept {
        Shape* shape = shape_create_circle(radius);
        if (shape == nullptr) {
            return Body();
        }

        RigidBody* raw = body_create(x, y, mass, shape);
        if (raw == nullptr) {
            shape_free(shape);
            return Body();
        }
        return Body(raw);
    }

    static Body create_box(float x, float y, float mass, float width, float height) noexcept {
        Shape* shape = shape_create_box(width, height);
        if (shape == nullptr) {
            return Body();
        }

        RigidBody* raw = body_create(x, y, mass, shape);
        if (raw == nullptr) {
            shape_free(shape);
            return Body();
        }
        return Body(raw);
    }

    bool valid() const noexcept {
        return handle_ != nullptr;
    }

    RigidBody* get() const noexcept {
        return handle_.get();
    }

    RigidBody* release() noexcept {
        return handle_.release();
    }

    void reset(RigidBody* raw = nullptr) noexcept {
        handle_.reset(raw);
    }

    void set_type(BodyType type) noexcept {
        body_set_type(handle_.get(), type);
    }

    void set_gravity(Vec2 gravity) noexcept {
        body_set_gravity(handle_.get(), gravity);
    }

    void apply_force(Vec2 force) noexcept {
        body_apply_force(handle_.get(), force);
    }

    void apply_impulse(Vec2 impulse) noexcept {
        body_apply_impulse(handle_.get(), impulse);
    }

private:
    Handle handle_;
};

}  // namespace physics2d

#endif
