#include "../include/physics2d/physics_world_raii.hpp"

int main() {
    physics2d::World world = physics2d::World::create();
    if (!world.valid()) {
        return 1;
    }

    world.step();
    physics2d::Status s = world.status();
    if (!s.ok()) {
        return 2;
    }
    return 0;
}
