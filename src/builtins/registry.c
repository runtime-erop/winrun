#include "winrun/builtin_registry.h"

#include <stddef.h>
#include <string.h>

typedef struct {
    const char *name;
    void *fn;
} entry;

static entry g_entries[512];
static size_t g_entry_count = 0;

void builtin_registry_register(const char *name, void *fn) {
    if (!name || !fn || g_entry_count >= (sizeof(g_entries) / sizeof(g_entries[0]))) {
        return;
    }
    g_entries[g_entry_count].name = name;
    g_entries[g_entry_count].fn = fn;
    ++g_entry_count;
}

void *builtin_registry_lookup(const char *name) {
    if (!name) {
        return NULL;
    }
    for (size_t i = 0; i < g_entry_count; ++i) {
        if (strcmp(name, g_entries[i].name) == 0) {
            return g_entries[i].fn;
        }
    }
    return NULL;
}
