#include "winrun/loader.h"

#include "winrun/abi.h"
#include "winrun/log.h"
#include "winrun/runtime_state.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) && defined(__linux__)
#include <asm/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

void set_loader_runtime_args(int argc, char **argv) {
    runtime_state_set_args(argc, argv);
}

int initialize_tls(const pe_image *image, mapped_image *mapped) {
    const char *enable_tls_callbacks = getenv("WINRUN_ENABLE_TLS_CALLBACKS");
    if (!enable_tls_callbacks || enable_tls_callbacks[0] == '\0' || strcmp(enable_tls_callbacks, "0") == 0) {
        return 0;
    }

    pe_data_directory tls_dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_TLS];
    if (tls_dir.virtual_address == 0 || tls_dir.size == 0) {
        return 0;
    }

    const uint32_t dll_process_attach = 1;

    if (image->is_pe64) {
        pe_tls_directory64 *tls = mapped_rva_to_ptr(mapped, tls_dir.virtual_address, sizeof(*tls));
        if (!tls) return -EINVAL;
        uint64_t callbacks_va = tls->address_of_callbacks;
        if (callbacks_va == 0) return 0;
        if (callbacks_va < mapped->actual_base) return -EINVAL;

        uint64_t callbacks_rva = callbacks_va - mapped->actual_base;
        uint64_t *callbacks = mapped_rva_to_ptr(mapped, (uint32_t)callbacks_rva, sizeof(uint64_t));
        if (!callbacks) return -EINVAL;

        for (; *callbacks != 0; ++callbacks) {
            if (*callbacks < mapped->actual_base) return -EINVAL;
            uint8_t *cb_ptr = mapped->base + (uint64_t)(*callbacks - mapped->actual_base);
            void (WINRUN_MS_ABI *tls_cb)(void *, uint32_t, void *) =
                (void (WINRUN_MS_ABI *)(void *, uint32_t, void *))(uintptr_t)cb_ptr;
            tls_cb((void *)(uintptr_t)mapped->actual_base, dll_process_attach, NULL);
        }
    } else {
        pe_tls_directory32 *tls = mapped_rva_to_ptr(mapped, tls_dir.virtual_address, sizeof(*tls));
        if (!tls) return -EINVAL;
        uint32_t callbacks_va = tls->address_of_callbacks;
        if (callbacks_va == 0) return 0;
        if ((uint64_t)callbacks_va < mapped->actual_base) return -EINVAL;

        uint32_t callbacks_rva = callbacks_va - (uint32_t)mapped->actual_base;
        uint32_t *callbacks = mapped_rva_to_ptr(mapped, callbacks_rva, sizeof(uint32_t));
        if (!callbacks) return -EINVAL;

        for (; *callbacks != 0; ++callbacks) {
            if ((uint64_t)(*callbacks) < mapped->actual_base) return -EINVAL;
            uint8_t *cb_ptr = mapped->base + (uint32_t)(*callbacks - (uint32_t)mapped->actual_base);
            void (WINRUN_MS_ABI *tls_cb)(void *, uint32_t, void *) =
                (void (WINRUN_MS_ABI *)(void *, uint32_t, void *))(uintptr_t)cb_ptr;
            tls_cb((void *)(uintptr_t)mapped->actual_base, dll_process_attach, NULL);
        }
    }

    return 0;
}

int execute_entry_point(const pe_image *image, mapped_image *mapped, int argc, char **argv) {
    (void)argc;
    (void)argv;
    uint8_t *entry = mapped->base + image->address_of_entry_point;
    int (WINRUN_MS_ABI *entry_fn)(void) = (int (WINRUN_MS_ABI *)(void))(uintptr_t)entry;

    log_info("jumping to entry point: 0x%llx", (unsigned long long)(mapped->actual_base + image->address_of_entry_point));
    int rc;
#if defined(__x86_64__) && defined(__linux__)
    unsigned long original_gs = 0;
    int have_original_gs = (syscall(SYS_arch_prctl, ARCH_GET_GS, &original_gs) == 0);
    uint8_t *fake_teb = calloc(1, 0x200);
    uint8_t *fake_peb = calloc(1, 0x200);
    if (fake_teb && fake_peb) {
        *(void **)(fake_teb + 0x30) = fake_peb;
        *(void **)(fake_peb + 0x08) = NULL;
        (void)syscall(SYS_arch_prctl, ARCH_SET_GS, (unsigned long)fake_teb);
    } else {
        log_warn("failed to allocate fake TEB/PEB; continuing without GS override");
    }
    rc = entry_fn();
    if (have_original_gs) {
        (void)syscall(SYS_arch_prctl, ARCH_SET_GS, original_gs);
    }
    free(fake_peb);
    free(fake_teb);
#else
    rc = entry_fn();
#endif
    return rc;
}
