#ifndef SCENE_BUILDER_H
#define SCENE_BUILDER_H

#include "physics.hpp"
#include "scene_catalog.hpp"

typedef int (*SceneOverlapResolver)(PhysicsEngine* engine, RigidBody* body, void* user);

void scene_builder_build(
    PhysicsEngine* engine,
    int scene_index,
    const SceneConfig* cfg,
    SceneOverlapResolver overlap_resolver,
    void* overlap_user);

#endif
