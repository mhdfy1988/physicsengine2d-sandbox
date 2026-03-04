#ifndef SNAPSHOT_REPO_H
#define SNAPSHOT_REPO_H

#include "physics.h"

int snapshot_repo_save(const PhysicsEngine* engine, const char* path);
int snapshot_repo_load(PhysicsEngine* engine, const char* path);

#endif
