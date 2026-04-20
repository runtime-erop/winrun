#ifndef WINRUN_ABI_H
#define WINRUN_ABI_H

#if defined(__x86_64__)
#define WINRUN_MS_ABI __attribute__((ms_abi))
#else
#define WINRUN_MS_ABI
#endif

#endif
