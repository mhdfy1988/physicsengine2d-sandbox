#ifndef PHYSICS_RUNTIME_APP_BRIDGE_HPP
#define PHYSICS_RUNTIME_APP_BRIDGE_HPP

#include "physics_runtime_facade.hpp"
#include "../apps/sandbox_dwrite/application/app_runtime.h"

namespace physics2d {
namespace runtime {

inline int app_error_code_from_runtime_error(RuntimeErrorCode code) noexcept {
    switch (code) {
        case RuntimeErrorCode::BridgeMissingReverse: return APP_RUNTIME_ERROR_CODE_BRIDGE_MISSING_REVERSE;
        case RuntimeErrorCode::BridgeStaleEntity: return APP_RUNTIME_ERROR_CODE_BRIDGE_STALE_ENTITY;
        case RuntimeErrorCode::BridgeNullBody: return APP_RUNTIME_ERROR_CODE_BRIDGE_NULL_BODY;
        case RuntimeErrorCode::BridgeDuplicateBody: return APP_RUNTIME_ERROR_CODE_BRIDGE_DUPLICATE_BODY;
        case RuntimeErrorCode::BridgeRefCountMismatch: return APP_RUNTIME_ERROR_CODE_BRIDGE_REFCOUNT_MISMATCH;
        case RuntimeErrorCode::PipelineMappingErrors: return APP_RUNTIME_ERROR_CODE_PIPELINE_MAPPING_ERRORS;
    }
    return APP_RUNTIME_ERROR_CODE_NONE;
}

inline int app_error_severity_from_runtime_error(RuntimeErrorSeverity severity) noexcept {
    return (severity == RuntimeErrorSeverity::Error) ? APP_RUNTIME_ERROR_ERROR : APP_RUNTIME_ERROR_WARNING;
}

inline int copy_runtime_errors_to_app_items(const std::vector<RuntimeError>& errors,
                                            AppRuntimeErrorItem* out_items,
                                            int out_cap) noexcept {
    int copied = 0;
    if (out_items == nullptr || out_cap <= 0) {
        return 0;
    }
    for (const RuntimeError& err : errors) {
        if (copied >= out_cap) {
            break;
        }
        out_items[copied].code = app_error_code_from_runtime_error(err.code);
        out_items[copied].severity = app_error_severity_from_runtime_error(err.severity);
        out_items[copied].count = static_cast<int>(err.count);
        copied++;
    }
    return copied;
}

inline void app_runtime_ingest_facade_errors(AppRuntime* runtime, const RuntimeFacade& facade) noexcept {
    AppRuntimeErrorItem items[APP_RUNTIME_MAX_ERRORS];
    const int n = copy_runtime_errors_to_app_items(facade.last_errors(), items, APP_RUNTIME_MAX_ERRORS);
    app_runtime_set_runtime_errors(runtime, items, n);
}

}  // namespace runtime
}  // namespace physics2d

#endif
