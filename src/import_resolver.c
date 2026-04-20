#include "winrun/loader.h"

#include "winrun/builtin_registry.h"
#include "winrun/log.h"

#include <ctype.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char dll[64];
    void *handle;
} dll_cache_entry;

static dll_cache_entry g_dll_cache[16];
static size_t g_dll_cache_size = 0;

static bool trust_system_libs_enabled(void) {
    const char *v = getenv("WINRUN_TRUST_SYSTEM_LIBS");
    return v && v[0] != '\0' && strcmp(v, "0") != 0;
}

static bool maybe_get_cached_dll(const char *dll, void **out_handle) {
    for (size_t i = 0; i < g_dll_cache_size; ++i) {
        if (strcmp(g_dll_cache[i].dll, dll) == 0) {
            *out_handle = g_dll_cache[i].handle;
            return true;
        }
    }
    return false;
}

static void cache_dll_handle(const char *dll, void *handle) {
    if (g_dll_cache_size >= (sizeof(g_dll_cache) / sizeof(g_dll_cache[0]))) {
        return;
    }
    snprintf(g_dll_cache[g_dll_cache_size].dll, sizeof(g_dll_cache[g_dll_cache_size].dll), "%s", dll);
    g_dll_cache[g_dll_cache_size].handle = handle;
    ++g_dll_cache_size;
}

static void normalize_dll_name(const char *dll, char *out, size_t out_size) {
    size_t j = 0;
    for (size_t i = 0; dll[i] != '\0' && j + 1 < out_size; ++i) {
        unsigned char ch = (unsigned char)dll[i];
        if (ch == '.') {
            break;
        }
        out[j++] = (char)tolower(ch);
    }
    out[j] = '\0';
}

static void *open_compatible_dll(const char *dll) {
    void *handle = NULL;
    if (maybe_get_cached_dll(dll, &handle)) {
        return handle;
    }

    char base[64];
    normalize_dll_name(dll, base, sizeof(base));
    if (base[0] == '\0') {
        cache_dll_handle(dll, NULL);
        return NULL;
    }

    char candidate[96];
    snprintf(candidate, sizeof(candidate), "lib%s.so", base);
    handle = dlopen(candidate, RTLD_NOW | RTLD_LOCAL);
    if (handle && getenv("WINRUN_DEBUG")) {
        log_debug("opened compatible system library %s for %s", candidate, dll);
    }

    cache_dll_handle(dll, handle);
    return handle;
}

static void *lookup_symbol_candidates(void *handle, const char *name) {
    if (!handle || !name || name[0] == '\0') {
        return NULL;
    }

    void *fn = dlsym(handle, name);
    if (fn) {
        return fn;
    }

    if (name[0] == '_') {
        fn = dlsym(handle, name + 1);
        if (fn) {
            return fn;
        }
    } else {
        char prefixed[256];
        if (snprintf(prefixed, sizeof(prefixed), "_%s", name) > 0) {
            fn = dlsym(handle, prefixed);
            if (fn) {
                return fn;
            }
        }
    }

    return NULL;
}

static void *resolve_symbol(import_resolver *resolver, const char *dll, const char *name) {
    void *builtin = builtin_registry_lookup(name);
    if (builtin) {
        return builtin;
    }

    if (resolver->default_winapi_lib) {
        void *fn = lookup_symbol_candidates(resolver->default_winapi_lib, name);
        if (fn) {
            return fn;
        }
    }

    if (trust_system_libs_enabled()) {
        void *dll_handle = open_compatible_dll(dll);
        if (dll_handle) {
            void *fn = lookup_symbol_candidates(dll_handle, name);
            if (fn) {
                return fn;
            }
        }

        void *default_fn = lookup_symbol_candidates(RTLD_DEFAULT, name);
        if (default_fn) {
            return default_fn;
        }
    }

    return NULL;
}

int resolve_imports(const pe_image *image, mapped_image *mapped, import_resolver *resolver) {
    pe_data_directory dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.virtual_address == 0) {
        return 0;
    }

    pe_import_descriptor *desc = mapped_rva_to_ptr(mapped, dir.virtual_address, dir.size);
    if (!desc) {
        return -EINVAL;
    }

    for (size_t d = 0; desc[d].name != 0; ++d) {
        pe_import_descriptor *cur = &desc[d];
        const char *dll_name = mapped_rva_to_ptr(mapped, cur->name, 1);
        if (!dll_name) {
            return -EINVAL;
        }

        uint64_t thunk_rva = cur->original_first_thunk ? cur->original_first_thunk : cur->first_thunk;
        if (thunk_rva == 0 || cur->first_thunk == 0) {
            continue;
        }

        if (image->is_pe64) {
            for (size_t i = 0;; ++i) {
                uint64_t *lk = mapped_rva_to_ptr(mapped, (uint32_t)thunk_rva + (uint32_t)(i * sizeof(uint64_t)), sizeof(uint64_t));
                uint64_t *out = mapped_rva_to_ptr(mapped, cur->first_thunk + (uint32_t)(i * sizeof(uint64_t)), sizeof(uint64_t));
                if (!lk || !out) {
                    return -EINVAL;
                }
                if (*lk == 0) {
                    break;
                }
                if ((*lk & (1ULL << 63)) != 0) {
                    continue;
                }

                uint32_t ibn_rva = (uint32_t)(*lk);
                uint8_t *import_by_name = mapped_rva_to_ptr(mapped, ibn_rva, 3);
                if (!import_by_name) {
                    return -EINVAL;
                }
                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    log_error("unresolved import: %s!%s", dll_name, func_name);
                    return -ENOENT;
                }
                *out = (uint64_t)(uintptr_t)fn;
            }
        } else {
            for (size_t i = 0;; ++i) {
                uint32_t *lk = mapped_rva_to_ptr(mapped, (uint32_t)thunk_rva + (uint32_t)(i * sizeof(uint32_t)), sizeof(uint32_t));
                uint32_t *out = mapped_rva_to_ptr(mapped, cur->first_thunk + (uint32_t)(i * sizeof(uint32_t)), sizeof(uint32_t));
                if (!lk || !out) {
                    return -EINVAL;
                }
                if (*lk == 0) {
                    break;
                }
                if ((*lk & (1U << 31)) != 0) {
                    continue;
                }

                uint8_t *import_by_name = mapped_rva_to_ptr(mapped, *lk, 3);
                if (!import_by_name) {
                    return -EINVAL;
                }
                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    log_error("unresolved import: %s!%s", dll_name, func_name);
                    return -ENOENT;
                }
                *out = (uint32_t)(uintptr_t)fn;
            }
        }
    }

    return 0;
}
