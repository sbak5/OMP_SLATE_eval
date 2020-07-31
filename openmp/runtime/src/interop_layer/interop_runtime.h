#ifndef INTEROP_H_
#define INTEROP_H_

#include "kmp.h"
#include "kmp_io.h"
extern cpu_set_t allCPU;

//#include "kmp_ftn_entry.h"
extern int (*real_sched_yield)();
extern thread_local int inOpenMP; //=0;

extern int interop_omp_init; //=0;
extern int argc; 
extern char **argv;

#include "interop_ext_comm.h"
#include <atomic>
#if HCLIB_OMP
#include "hclib.h"
#include "hclib_omp.h"
//#define interop_thread_local(x) x[hclib_get_current_worker()]
#define interop_thread_local_other(x, id) x[id]
#elif CHARM_OMP || CONV_OMP
#define interop_thread_local(x) CpvAccess(x)
#define interop_thread_local_other(x, id) CpvAccessOther(x, id)
#include "ompcharm.h"
#include <sys/sysinfo.h>
#include <math.h>
typedef CthThread kmp_thread_t;
typedef pthread_key_t kmp_key_t;
extern kmp_key_t __kmp_curPE;

//#define CpvAccess(v) CpvAccessOther(v, *(int*)pthread_getspecific(__kmp_curPE))
#if CONV_OMP
extern "C" void CmiInitCPUTopology(char **argv);
extern "C" void CmiCheckAffinity();
extern "C" void CmiInitCPUAffinity(char **argv);
extern "C" void CmiInitMemAffinity(char **argv);

#define CmiGetStateN(n) (Cmi_state_vector+(n))  
/*extern kmp_mutex_align_t __kmp_uber_mutex;
extern kmp_cond_align_t __kmp_uber_cond;
extern kmp_cond_align_t __kmp_init_cond;
extern kmp_mutex_align_t __kmp_init_mutex;
*/
//extern pthread_t __kmp_root_uber_thread;
extern kmp_int32 __kmp_initial_root_exit;
//#include <vector>
//#include <dlfcn.h>
//extern std::vector<kmp_int32> __kmp_core_id_list;
extern kmp_int32 __kmp_core_id_idx;
#endif
extern void* __kmp_launch_worker(void *);
#endif
#include "interop_thread.h"
void interop_initialize(); //int argc, char *argv);

void interop_var_initialize();

void interop_root_initialize(void *root_thread, int gtid, int initial_thread); 
void interop_post_initialize(); //int argc, char *argv);

void interop_suspend_worker(kmp_int32 gtid, kmp_team_t *parent_team);

void interop_release_worker();

void interop_signal_finalize(); 

void interop_finalize();

void interop_thread_finalize(int gtid, int prevCoreID, int gtid_req);


void interop_master_wait(kmp_info_t *th, volatile kmp_int32 *barrier_ptr, kmp_int32 reset_val, int sched_task=true);

void *interop_create_task(void (*func)(void*), kmp_task_t *task);

void interop_schedule_task(void *interop_task);

void *interop_pop_task();

void *interop_steal_task();

void interop_execute_task(void*);

void interop_execute_gang_task();

// inline functions to create / resume / reap user-level threads
inline void interop_create_thread(void* (*start_func)(void *), kmp_info_t *th, int stack_size) {
#if HCLIB_OMP
  th->th.th_info.ds.ds_thread = hclib_async_ult((generic_frame_ptr)start_func, th, NULL, 0, NULL, stack_size);
#elif CHARM_OMP || CONV_OMP
  th->th.th_info.ds.ds_thread = CthCreate((CthVoidFn)start_func, th, stack_size);
  if (th->th.th_info.ds.ds_tid == 0)
    CthSetSuspendable(th->th.th_info.ds.ds_thread,0);

  CthSetStrategyWorkStealing(th->th.th_info.ds.ds_thread);
#endif
  KA_TRACE(10, ("Thread created: %p\n", th->th.th_info.ds.ds_thread));
  return;
}

inline void interop_reap_thread(kmp_info_t *th) {
#if HCLIB_OMP
  LiteCtx_destroy(th->th.th_info.ds.ds_thread);
#elif CHARM_OMP || CONV_OMP
  CthFree(th->th.th_info.ds.ds_thread);
#endif
}

inline void interop_resume_thread(kmp_info_t *th) { 
#if HCLIB_OMP
//  hclib_locale_t *locale = hclib_get_assigned_worker(); // Return locale of workers assigned in interop_reserve_workers
  hclib_resume_ult(th->th.th_info.ds.ds_thread, NULL, 0);
#elif CHARM_OMP || CONV_OMP
  CthAwaken(th->th.th_info.ds.ds_thread);
#endif
}

inline int interop_get_num_workers() {
#if HCLIB_OMP
  return hclib_get_num_workers();
#elif CHARM_OMP || CONV_OMP
  return CmiMyNodeSize();
#endif
}

inline int interop_get_worker_id () {
#if HCLIB_OMP
  return hclib_get_current_worker();
#elif CHARM_OMP || CONV_OMP
  return CmiMyRank();
#endif
}

inline int interop_get_dynamic_workers (int num_request) {
#if HCLIB_OMP
  return hclib_get_dynamic_workers(num_request);
#endif
}

inline void interop_set_worker_ntasks(int worker_ntasks, int residual_ntasks) {
#if HCLIB_OMP
  return hclib_set_worker_ntasks(worker_ntasks, residual_ntasks);
#endif
}


template<typename T> inline T& interop_thread_local(T* var) {
  return var[interop_get_worker_id()]; 
}

inline int& interop_thread_local(std::vector<int> **var) {
  return var[interop_get_worker_id()]->at(interop_thread_local(nest_level)); 
}

inline void interop_set_global_variable(int gtid, int enter, int jump) {
//  int *global_id;
//#if HCLIB_OMP

//#elif CHARM || CONV_OMP 
  KA_TRACE(4, ("interop_set_global_variable: [%d] T#%d gtid will change its gtid, prevGtid: %d, enter: %d, jump: %d \n", interop_get_worker_id(), gtid, interop_thread_local(prevGtid), enter, jump));
//#endif 
  if (enter) {
    interop_thread_local(nest_level)++;
    int &global_id = interop_thread_local(prevGtid);
    if (!jump) {
#if KMP_TDATA_GTID
      global_id = __kmp_gtid;
#endif
      global_id = __kmp_gtid_get_specific();
    }
#if KMP_TDATA_GTID
    __kmp_gtid = gtid;
#endif
    __kmp_gtid_set_specific(gtid);
    inOpenMP = 1;
  } else {
    int &global_id = interop_thread_local(prevGtid);
    if (global_id >=0) {
#if KMP_TDATA_GTID
      __kmp_gtid = global_id;
#endif
      __kmp_gtid_set_specific(global_id);
    }
    if (interop_thread_local(nest_level) > 0)
        interop_thread_local(nest_level)--;
    inOpenMP = 0;
  }
  HASSERT(interop_thread_local(nest_level) < INIT_NEST_LEVEL && interop_thread_local(nest_level) >=0);

  KA_TRACE(4, ("interop_set_global_variable: [%d] T#%d gtid changed to its gtid %d, prevGtid: %d, enter: %d, jump: %d \n", interop_get_worker_id(), gtid,__kmp_get_gtid(), interop_thread_local(prevGtid), enter, jump));

  KMP_MB();
}

inline void interop_suspend_thread(int th_gtid) {
#if KMP_DEBUG 
  int cur_gtid = interop_thread_local(prevGtid);
  KF_TRACE( 5, ( "__kmp_suspend_template: [%d] T#%d gtid will be restored to: %d, %d \n",interop_get_worker_id(),
                     th_gtid, cur_gtid, __kmp_gtid_get_specific()) );
#endif
  interop_set_global_variable(th_gtid, 0, 0);
/*#if HCLIB_OMP
#elif CHARM_OMP || CONV_OMP
  cur_gtid = interop_thread_local(prevGtid);
#endif
  if (cur_gtid >=0) {
    KF_TRACE( 5, ( "__kmp_suspend_template: [%d] T#%d gtid will be restored to: %d, %d \n",interop_get_worker_id(),
                     th_gtid, cur_gtid, __kmp_gtid_get_specific()) );
#if KMP_TDATA_GTID
    __kmp_gtid = cur_gtid;
#endif
    __kmp_gtid_set_specific(cur_gtid);
  }
  //th->th.th_reap_state = KMP_SAFE_TO_REAP;
  inOpenMP=0;*/
#if HCLIB_OMP
  hclib_suspend_ult();
#elif CHARM_OMP || CONV_OMP
  CthSuspend();
#endif
  interop_set_global_variable(th_gtid, 1, 0);
/*
  inOpenMP=1;
  __kmp_gtid_set_specific(th_gtid);
#if KMP_TDATA_GTID
  __kmp_gtid = th_gtid;
#endif*/
  KF_TRACE( 5, ( "__kmp_suspend_template: T#%d is resumed \n",
                     th_gtid) );
 // th->th.th_reap_state = KMP_NOT_SAFE_TO_REAP;
}

inline void interop_thread_scheduled(kmp_thread_t *thread, kmp_int32 nproc) {
  int sched;
  for (int i = 1; i < nproc ; i++) {
    do {
        std::atomic_thread_fence(std::memory_order_acquire);
#if HCLIB_OMP
      sched = TCR_4(thread[i]->scheduled);
#elif CHARM_OMP || CONV_OMP
      sched = TCR_4(CthScheduled(thread[i])); // CthScheduled(t));
#endif
//      __kmp_printf("thread[%d]->scheduled is %d\n", i, thread[i]->scheduled);
//      __sync_synchronize();
    } while (sched > 0);
  }
}

inline void* interop_get_worker_state() {
#if HCLIB_OMP
  return CURRENT_WS_INTERNAL; 
#elif CHARM_OMP || CONV_OMP 
  return CmiGetStateExt();
#endif
}

inline void interop_steal_worker_state(int rank) {
#if HCLIB_OMP
  set_current_worker(rank);
#elif CHARM_OMP || CONV_OMP
  CmiStealState(rank);
#endif
}

inline bool interop_master_running_status() {
//#if HCLIB_OMP
//#elif CHARM_OMP || CONV_OMP
  return interop_thread_local(masterRunning);
//#endif
}

inline void interop_reserve_workers (int num_workers, int nest_level) {
  if (nest_level == 0 || __kmp_threads[__kmp_get_gtid()]->th.th_current_task->td_icvs.gang_enabled)
      hclib_reserve_workers(num_workers, nest_level);
}

inline void interop_reserve_workers_post() {
  kmp_info_t *thread = __kmp_threads[__kmp_get_gtid()];
  if (thread->th.th_team->t.t_parent->t.t_level == 0 || thread->th.th_current_task->td_icvs.gang_enabled)
      hclib_reserve_workers_post();
}

inline void interop_release_workers() { //int num_request, int nest_level) {
  // Release a list of workers for gang scheduling
  hclib_release_workers(); //num_request, nest_level);
  // Wake up suspended worker
  interop_release_worker();
}


#include "interop_runtime_inline.h"
#endif // INTEROP_H_
