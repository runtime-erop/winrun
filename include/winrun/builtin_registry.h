#ifndef WINRUN_BUILTIN_REGISTRY_H
#define WINRUN_BUILTIN_REGISTRY_H

void builtin_registry_register(const char *name, void *fn);
void *builtin_registry_lookup(const char *name);

#endif
