#include "scene_builder.hpp"

static void add_static_world(PhysicsEngine* engine) {
    Shape* ground_shape = shape_create_box(120.0f, 2.0f);
    ground_shape->friction = 0.85f;
    ground_shape->restitution = 0.0f;
    RigidBody* ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
    body_set_type(ground, BODY_STATIC);
    physics_engine_add_body(engine, ground);

    Shape* left_wall_shape = shape_create_box(2.0f, 60.0f);
    left_wall_shape->friction = 0.75f;
    RigidBody* left_wall = body_create(0.0f, 25.0f, 1000.0f, left_wall_shape);
    body_set_type(left_wall, BODY_STATIC);
    physics_engine_add_body(engine, left_wall);
}

static void scene_default_bounce(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    for (i = 0; i < 3; i++) {
        Shape* ball_shape = shape_create_circle(2.0f);
        ball_shape->restitution = cfg->ball_restitution;
        RigidBody* ball = body_create(40.0f + i * 8.0f, 10.0f, cfg->ball_mass, ball_shape);
        ball->velocity = vec2((i == 1) ? -8.0f : 8.0f, 0.0f);
        physics_engine_add_body(engine, ball);
    }
    for (i = 0; i < 2; i++) {
        Shape* box_shape = shape_create_box(8.0f, 8.0f);
        box_shape->restitution = cfg->box_restitution;
        RigidBody* box = body_create(30.0f + i * 35.0f, 18.0f, cfg->box_mass, box_shape);
        box->angular_velocity = (i == 0) ? 2.0f : -1.5f;
        physics_engine_add_body(engine, box);
    }
}

static void add_dynamic_body_safe(PhysicsEngine* engine, RigidBody* body, SceneOverlapResolver overlap_resolver, void* overlap_user) {
    if (engine == 0 || body == 0) return;
    physics_engine_add_body(engine, body);
    if (overlap_resolver != 0) {
        if (!overlap_resolver(engine, body, overlap_user)) {
            physics_engine_remove_body(engine, body);
        }
    }
}

static void scene_high_speed_tunnel(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* thin_shape = shape_create_box(1.2f, 24.0f);
    RigidBody* thin_wall = body_create(58.0f, 28.0f, 1000.0f, thin_shape);
    body_set_type(thin_wall, BODY_STATIC);
    physics_engine_add_body(engine, thin_wall);

    for (i = 0; i < 4; i++) {
        Shape* target_box_shape = shape_create_box(4.2f, 4.2f);
        target_box_shape->restitution = cfg->box_restitution;
        physics_engine_add_body(engine, body_create(74.0f + i * 5.0f, 38.0f - i * 4.4f, cfg->box_mass, target_box_shape));
    }

    for (i = 0; i < 2; i++) {
        Shape* fast_ball_shape = shape_create_circle(1.0f);
        RigidBody* fast_ball;
        fast_ball_shape->friction = 0.05f;
        fast_ball_shape->restitution = cfg->ball_restitution;
        fast_ball = body_create(10.0f, 30.0f + i * 4.0f, cfg->ball_mass, fast_ball_shape);
        fast_ball->velocity = vec2(105.0f + i * 22.0f, -4.0f + i * 2.0f);
        physics_engine_add_body(engine, fast_ball);
    }
}

static void scene_mass_ratio(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* heavy_box_shape = shape_create_box(10.0f, 10.0f);
    RigidBody* heavy_box;
    heavy_box_shape->restitution = cfg->box_restitution;
    heavy_box_shape->friction = 0.45f;
    heavy_box = body_create(72.0f, 36.0f, cfg->box_mass, heavy_box_shape);
    physics_engine_add_body(engine, heavy_box);

    for (i = 0; i < 10; i++) {
        Shape* light_ball_shape = shape_create_circle(1.5f);
        RigidBody* light_ball;
        light_ball_shape->restitution = cfg->ball_restitution;
        light_ball_shape->friction = 0.15f;
        light_ball = body_create(12.0f + i * 3.5f, 18.2f + (i % 2) * 1.8f, cfg->ball_mass, light_ball_shape);
        light_ball->velocity = vec2(26.0f + (i % 3) * 2.5f, 0.0f);
        physics_engine_add_body(engine, light_ball);
    }
}

static void scene_high_stack(PhysicsEngine* engine, const SceneConfig* cfg) {
    int c;
    int i;
    for (c = 0; c < 3; c++) {
        for (i = 0; i < 12; i++) {
            Shape* box_shape = shape_create_box(4.6f, 4.6f);
            box_shape->friction = 0.38f;
            box_shape->restitution = cfg->box_restitution;
            physics_engine_add_body(engine, body_create(38.0f + c * 12.0f, 43.0f - i * 4.8f, cfg->box_mass, box_shape));
        }
    }

    {
        Shape* striker_shape = shape_create_circle(3.2f);
        RigidBody* striker;
        striker_shape->restitution = cfg->ball_restitution;
        striker_shape->friction = 0.2f;
        striker = body_create(14.0f, 39.0f, cfg->ball_mass * 5.0f, striker_shape);
        striker->velocity = vec2(36.0f, 0.0f);
        physics_engine_add_body(engine, striker);
    }
}

static void scene_friction_slope(PhysicsEngine* engine, const SceneConfig* cfg, SceneOverlapResolver overlap_resolver, void* overlap_user) {
    int i;
    Shape* ramp_shape = shape_create_box(56.0f, 2.5f);
    RigidBody* ramp = body_create(36.0f, 28.0f, 1000.0f, ramp_shape);
    body_set_type(ramp, BODY_STATIC);
    ramp->angle = -0.42f;
    physics_engine_add_body(engine, ramp);

    for (i = 0; i < 5; i++) {
        Shape* ball_shape = shape_create_circle(1.7f);
        ball_shape->restitution = cfg->ball_restitution;
        ball_shape->friction = 0.03f + i * 0.12f;
        add_dynamic_body_safe(engine, body_create(12.0f + i * 5.0f, 8.0f + i * 1.1f, cfg->ball_mass, ball_shape), overlap_resolver, overlap_user);
    }
    for (i = 0; i < 4; i++) {
        Shape* box_shape = shape_create_box(4.2f, 4.2f);
        box_shape->restitution = cfg->box_restitution;
        box_shape->friction = 0.10f + i * 0.18f;
        add_dynamic_body_safe(engine, body_create(18.0f + i * 8.0f, 11.0f + i * 1.2f, cfg->box_mass, box_shape), overlap_resolver, overlap_user);
    }
}

static void scene_restitution_matrix(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    const float rvals[3] = {0.20f, 0.55f, 0.90f};
    for (i = 0; i < 3; i++) {
        Shape* left_shape = shape_create_circle(1.8f);
        Shape* right_shape = shape_create_circle(1.8f);
        RigidBody* left_ball;
        RigidBody* right_ball;
        left_shape->restitution = rvals[i];
        right_shape->restitution = rvals[i];
        left_shape->friction = 0.1f;
        right_shape->friction = 0.1f;
        left_ball = body_create(24.0f, 16.0f + i * 8.0f, cfg->ball_mass, left_shape);
        right_ball = body_create(76.0f, 16.0f + i * 8.0f, cfg->ball_mass, right_shape);
        left_ball->velocity = vec2(14.0f + i * 2.0f, 0.0f);
        right_ball->velocity = vec2(-14.0f - i * 2.0f, 0.0f);
        physics_engine_add_body(engine, left_ball);
        physics_engine_add_body(engine, right_ball);
    }
}

static void scene_drag_stress(PhysicsEngine* engine, const SceneConfig* cfg) {
    int x;
    int y;
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 9; x++) {
            Shape* ball_shape = shape_create_circle(1.9f);
            ball_shape->restitution = cfg->ball_restitution;
            ball_shape->friction = 0.2f;
            physics_engine_add_body(engine, body_create(20.0f + x * 5.0f, 20.0f + y * 3.6f, cfg->ball_mass, ball_shape));
        }
    }
    for (x = 0; x < 8; x++) {
        Shape* box_shape = shape_create_box(4.8f, 4.8f);
        box_shape->restitution = cfg->box_restitution;
        box_shape->friction = 0.35f;
        physics_engine_add_body(engine, body_create(18.0f + x * 9.0f, 11.0f + (x % 2) * 2.5f, cfg->box_mass, box_shape));
    }
}

static void scene_boundary_squeeze(PhysicsEngine* engine, const SceneConfig* cfg) {
    int i;
    Shape* ground_shape;
    RigidBody* ground;
    ground_shape = shape_create_box(120.0f, 2.0f);
    ground_shape->friction = 0.85f;
    ground_shape->restitution = 0.0f;
    ground = body_create(50.0f, 45.0f, 1000.0f, ground_shape);
    body_set_type(ground, BODY_STATIC);
    physics_engine_add_body(engine, ground);

    for (i = 0; i < 14; i++) {
        Shape* shot_shape = shape_create_circle(1.6f);
        RigidBody* shot_ball;
        shot_shape->restitution = cfg->ball_restitution;
        shot_shape->friction = 0.05f;
        shot_ball = body_create(8.0f + (i % 3) * 3.0f, 8.0f + (float)i * 2.4f, cfg->ball_mass, shot_shape);
        shot_ball->velocity = vec2(30.0f + (i % 4) * 4.0f, 0.0f);
        physics_engine_add_body(engine, shot_ball);
    }
}

static void scene_performance_limit(PhysicsEngine* engine, const SceneConfig* cfg) {
    int x;
    int y;
    for (y = 0; y < 10; y++) {
        for (x = 0; x < 16; x++) {
            Shape* ball_shape = shape_create_circle(1.2f);
            ball_shape->restitution = cfg->ball_restitution;
            ball_shape->friction = 0.12f;
            physics_engine_add_body(engine, body_create(8.0f + x * 5.1f, 6.0f + y * 3.8f, cfg->ball_mass, ball_shape));
        }
    }
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 10; x++) {
            Shape* box_shape = shape_create_box(3.4f, 3.4f);
            box_shape->restitution = cfg->box_restitution;
            box_shape->friction = 0.25f;
            physics_engine_add_body(engine, body_create(11.0f + x * 7.8f, 7.5f + y * 6.0f, cfg->box_mass, box_shape));
        }
    }
}

void scene_builder_build(
    PhysicsEngine* engine,
    int scene_index,
    const SceneConfig* cfg,
    SceneOverlapResolver overlap_resolver,
    void* overlap_user) {
    if (engine == 0 || cfg == 0) return;

    if (scene_index != 7) {
        add_static_world(engine);
    }

    switch (scene_index) {
        case 1:
            scene_high_speed_tunnel(engine, cfg);
            break;
        case 2:
            scene_mass_ratio(engine, cfg);
            break;
        case 3:
            scene_high_stack(engine, cfg);
            break;
        case 4:
            scene_friction_slope(engine, cfg, overlap_resolver, overlap_user);
            break;
        case 5:
            scene_restitution_matrix(engine, cfg);
            break;
        case 6:
            scene_drag_stress(engine, cfg);
            break;
        case 7:
            scene_boundary_squeeze(engine, cfg);
            break;
        case 8:
            scene_performance_limit(engine, cfg);
            break;
        default:
            scene_default_bounce(engine, cfg);
            break;
    }
}
