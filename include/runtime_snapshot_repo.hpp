#ifndef RUNTIME_SNAPSHOT_REPO_H
#define RUNTIME_SNAPSHOT_REPO_H

#include "physics.hpp"

#ifdef __cplusplus
extern "C" {
#endif

int runtime_snapshot_repo_save(const PhysicsEngine* engine, const char* path);
int runtime_snapshot_repo_load(PhysicsEngine* engine, const char* path);

#ifdef __cplusplus
}
#endif

#endif
