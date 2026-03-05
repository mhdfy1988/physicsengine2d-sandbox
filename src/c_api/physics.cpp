#include <string.h>
#include "physics_internal.h"

void physics_internal_set_error(PhysicsEngine* engine, PhysicsErrorCode error, const char* message_override) {
    if (engine == NULL) return;
    engine->last_error = error;
    if (error != PHYSICS_ERROR_NONE) {
        const char* message = message_override != NULL ? message_override : physics_error_code_string(error);
        physics_internal_emit_event(engine, PHYSICS_EVENT_ERROR, (int)error, 0.0, message, NULL);
    }
}

static void physics_emit_trace_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                     const char* message, const CollisionManifold* contact) {
#if PHYSICS_ENABLE_TRACE
    PhysicsTraceEvent event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.step_index = engine->last_profile.step_index;
    switch (type) {
        case PHYSICS_EVENT_POST_BROADPHASE:
        case PHYSICS_EVENT_POST_NARROWPHASE:
            event.payload.stage.count = ivalue;
            event.payload.stage.elapsed_ms = dvalue;
            break;
        case PHYSICS_EVENT_STEP_END:
            event.payload.step_end.contact_count = ivalue;
            event.payload.step_end.total_ms = dvalue;
            break;
        case PHYSICS_EVENT_ERROR:
            event.payload.error.code = (PhysicsErrorCode)ivalue;
            event.payload.error.message = message;
            break;
        case PHYSICS_EVENT_CONTACT_CREATED:
            event.payload.contact.contact = contact;
            break;
        default:
            break;
    }
    if (engine->event_sink != NULL) {
        engine->event_sink(&event, engine->event_sink_user);
    }
#else
    (void)engine;
    (void)type;
    (void)ivalue;
    (void)dvalue;
    (void)message;
    (void)contact;
#endif
}

static void physics_emit_legacy_callbacks(PhysicsEngine* engine, PhysicsEventType type, int ivalue,
                                          const CollisionManifold* contact) {
    /* Compatibility bridge for legacy callbacks (always active for backward compatibility). */
    switch (type) {
        case PHYSICS_EVENT_STEP_BEGIN:
            if (engine->callbacks.on_pre_step != NULL) {
                engine->callbacks.on_pre_step(engine, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_POST_BROADPHASE:
            if (engine->callbacks.on_post_broadphase != NULL) {
                engine->callbacks.on_post_broadphase(engine, ivalue, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_POST_NARROWPHASE:
            if (engine->callbacks.on_post_narrowphase != NULL) {
                engine->callbacks.on_post_narrowphase(engine, ivalue, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_CONTACT_CREATED:
            if (engine->callbacks.on_contact_created != NULL && contact != NULL) {
                engine->callbacks.on_contact_created(engine, contact, engine->callbacks.user);
            }
            break;
        case PHYSICS_EVENT_STEP_END:
            if (engine->callbacks.on_post_step != NULL) {
                engine->callbacks.on_post_step(engine, &engine->last_profile, engine->callbacks.user);
            }
            break;
        default:
            break;
    }
}

void physics_internal_emit_event(PhysicsEngine* engine, PhysicsEventType type, int ivalue, double dvalue,
                                 const char* message, const CollisionManifold* contact) {
    if (engine == NULL) return;
    physics_emit_trace_event(engine, type, ivalue, dvalue, message, contact);
    physics_emit_legacy_callbacks(engine, type, ivalue, contact);
}
