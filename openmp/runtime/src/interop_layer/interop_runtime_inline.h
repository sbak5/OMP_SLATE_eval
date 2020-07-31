#ifndef INTEROP_INLINE_H_
#define INTEROP_INLINE_H_
#include "kmp.h"
extern void interop_create_thread(void* (*start_func)(void*), kmp_info_t *th, int stack_size);
extern bool interop_master_running_status();
extern void interop_resume_thread(kmp_info_t *th);
extern void interop_suspend_thread(int th_gtid);
extern void interop_reap_thread(kmp_info_t *th);
extern void interop_thread_scheduled(kmp_thread_t *thread, kmp_int32 nproc);
extern int interop_get_num_workers();
extern int interop_get_worker_id ();
extern void interop_set_global_variable(int gtid, int enter, int jump);

extern void* interop_get_worker_state(); 
extern void interop_steal_worker_state(int rank);
#endif
