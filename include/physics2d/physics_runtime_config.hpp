#ifndef PHYSICS_RUNTIME_CONFIG_HPP
#define PHYSICS_RUNTIME_CONFIG_HPP

#include <string>
#include "physics_filesystem.hpp"

namespace physics2d {
namespace foundation {

struct RuntimePathConfig {
    Path project_root;
    Path assets_dir;
    Path scenes_dir;
    Path prefabs_dir;
    Path packages_dir;
    Path project_settings_dir;
    Path build_dir;
    Path cache_dir;

    static RuntimePathConfig from_project_root(const Path& root) {
        RuntimePathConfig config;
        config.project_root = normalize_path(root);
        config.assets_dir = config.project_root / "Assets";
        config.scenes_dir = config.project_root / "Scenes";
        config.prefabs_dir = config.project_root / "Prefabs";
        config.packages_dir = config.project_root / "Packages";
        config.project_settings_dir = config.project_root / "ProjectSettings";
        config.build_dir = config.project_root / "Build";
        config.cache_dir = config.project_root / "Cache";
        return config;
    }

    bool ensure_minimal_tree() const {
        return ensure_directory(project_root) &&
               ensure_directory(assets_dir) &&
               ensure_directory(scenes_dir) &&
               ensure_directory(prefabs_dir) &&
               ensure_directory(packages_dir) &&
               ensure_directory(project_settings_dir) &&
               ensure_directory(build_dir) &&
               ensure_directory(cache_dir);
    }

    bool valid() const {
        return !project_root.empty() &&
               !assets_dir.empty() &&
               !scenes_dir.empty() &&
               !prefabs_dir.empty() &&
               !packages_dir.empty() &&
               !project_settings_dir.empty() &&
               !build_dir.empty() &&
               !cache_dir.empty();
    }
};

}  // namespace foundation
}  // namespace physics2d

#endif
