#include "winrun/loader.h"

#include "winrun/log.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char *exe_path;
    pe_image image;
    mapped_image mapped;
    void *winapi;
} loader_context;

static bool env_debug_enabled(void) {
    const char *debug = getenv("WINRUN_DEBUG");
    return debug != NULL && strcmp(debug, "0") != 0;
}

static const char *resolve_winapi_lib_path(void) {
    const char *env_path = getenv("WINRUN_WINAPI_LIB");
    if (env_path && env_path[0] != '\0') {
        return env_path;
    }

    static const char *candidates[] = {
        "./lib/libwinapi.so",
        "/usr/lib/winrun/libwinapi.so",
        "/usr/lib/libwinapi.so",
    };
    for (size_t i = 0; i < (sizeof(candidates) / sizeof(candidates[0])); ++i) {
        if (access(candidates[i], R_OK) == 0) {
            return candidates[i];
        }
    }
    return "./lib/libwinapi.so";
}

static void warn_if_missing_runtime_env(bool debug) {
    const char *xdg_runtime = getenv("XDG_RUNTIME_DIR");
    if (xdg_runtime && xdg_runtime[0] != '\0') {
        return;
    }

    if (geteuid() == 0) {
        const char *sudo_uid = getenv("SUDO_UID");
        if (sudo_uid && sudo_uid[0] != '\0') {
            char guessed_runtime[128];
            snprintf(guessed_runtime, sizeof(guessed_runtime), "/run/user/%s", sudo_uid);
            if (access(guessed_runtime, F_OK) == 0 && setenv("XDG_RUNTIME_DIR", guessed_runtime, 1) == 0) {
                log_warn("using inferred XDG_RUNTIME_DIR=%s from SUDO_UID", guessed_runtime);
                return;
            }
        }
        log_warn("XDG_RUNTIME_DIR is not set for root. Preserve desktop vars when using sudo.");
        return;
    }

    if (debug) {
        log_warn("XDG_RUNTIME_DIR is not set; desktop input backends may be unavailable");
    }
}

static char *normalize_windows_exe_path(const char *raw_path) {
    size_t len = strlen(raw_path);
    char *normalized = malloc(len + 1);
    if (!normalized) {
        return NULL;
    }

    size_t out = 0;
    for (size_t i = 0; i < len; ++i) {
        if (raw_path[i] == '\\' && i + 1 < len && raw_path[i + 1] == ' ') {
            normalized[out++] = ' ';
            ++i;
            continue;
        }
        normalized[out++] = raw_path[i];
    }
    normalized[out] = '\0';
    return normalized;
}

static void cleanup(loader_context *ctx) {
    if (ctx->mapped.base) {
        unmap_pe_image(&ctx->mapped);
    }
    if (ctx->image.file_data) {
        pe_destroy_image(&ctx->image);
    }
    if (ctx->winapi) {
        dlclose(ctx->winapi);
    }
    free(ctx->exe_path);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <windows-exe> [args...]\n", argv[0]);
        return 2;
    }

    bool debug = env_debug_enabled();
    log_set_debug(debug);
    install_signal_handlers(debug);
    warn_if_missing_runtime_env(debug);

    loader_context ctx = {0};
    ctx.exe_path = normalize_windows_exe_path(argv[1]);
    if (!ctx.exe_path) {
        log_error("out of memory while preparing executable path");
        return 1;
    }

    int rc = pe_parse_image(ctx.exe_path, &ctx.image);
    if (rc != 0) {
        log_error("failed to parse PE image '%s' (rc=%d)", ctx.exe_path, rc);
        cleanup(&ctx);
        return 1;
    }

    rc = map_pe_image(&ctx.image, &ctx.mapped);
    if (rc != 0) {
        log_error("failed to map image (rc=%d)", rc);
        cleanup(&ctx);
        return 1;
    }

    rc = apply_relocations(&ctx.image, &ctx.mapped);
    if (rc != 0 && ctx.mapped.actual_base != ctx.mapped.preferred_base) {
        log_error("failed to apply relocations (rc=%d)", rc);
        cleanup(&ctx);
        return 1;
    }

    const char *winapi_lib_path = resolve_winapi_lib_path();
    ctx.winapi = dlopen(winapi_lib_path, RTLD_NOW | RTLD_LOCAL);
    if (!ctx.winapi) {
        log_error("failed to load WinAPI library '%s': %s", winapi_lib_path, dlerror());
        cleanup(&ctx);
        return 1;
    }
    log_debug("loaded WinAPI shim from %s", winapi_lib_path);

    set_loader_runtime_args(argc - 1, &argv[1]);

    import_resolver resolver = {
        .default_winapi_lib = ctx.winapi,
        .debug = debug,
    };

    rc = resolve_imports(&ctx.image, &ctx.mapped, &resolver);
    if (rc != 0) {
        cleanup(&ctx);
        return 1;
    }

    rc = initialize_tls(&ctx.image, &ctx.mapped);
    if (rc != 0) {
        log_error("failed to initialize TLS (rc=%d)", rc);
        cleanup(&ctx);
        return 1;
    }

    rc = protect_pe_image_sections(&ctx.image, &ctx.mapped);
    if (rc != 0) {
        log_error("failed to apply section protections (rc=%d)", rc);
        cleanup(&ctx);
        return 1;
    }

    int app_rc = execute_entry_point(&ctx.image, &ctx.mapped, argc - 1, &argv[1]);
    cleanup(&ctx);
    return app_rc;
}
