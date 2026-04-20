#ifndef WINRUN_RUNTIME_STATE_H
#define WINRUN_RUNTIME_STATE_H

#include <stddef.h>
#include <stdint.h>

void runtime_state_set_args(int argc, char **argv);
int *runtime_state_argc_ptr(void);
char ***runtime_state_argv_ptr(void);
char ***runtime_state_environ_ptr(void);
char ***runtime_state_initenv_ptr(void);
int *runtime_state_fmode_ptr(void);
int *runtime_state_commode_ptr(void);
unsigned int runtime_state_lc_codepage(void);
int runtime_state_mb_cur_max(void);
uint32_t runtime_state_get_last_error(void);
void runtime_state_set_last_error(uint32_t value);
void runtime_state_push_onexit(void (*fn)(void));
void runtime_state_run_onexit(void);

#endif
