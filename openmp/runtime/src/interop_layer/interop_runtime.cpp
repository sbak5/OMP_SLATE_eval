#include "interop_runtime.h"
#include <sys/sysinfo.h>
//#include "kmp.h"
#include "kmp_io.h"
#include <algorithm>
#include <vector>
int (*real_sched_yield)() = NULL;

int interop_omp_init=0;
std::atomic<int> interop_init_workers;
int first_worker_early_exit=0;
int argc; 
char **argv;
int num_workers=1;

cpu_set_t allCPU;
kmp_mutex_align_t __kmp_uber_mutex;
kmp_cond_align_t __kmp_uber_cond;
kmp_cond_align_t __kmp_init_cond;
kmp_mutex_align_t __kmp_init_mutex;

pthread_t __kmp_root_uber_thread;
kmp_int32 __kmp_initial_root_exit;
std::vector<kmp_int32> __kmp_core_id_list;
kmp_int32 __kmp_core_id_idx;
//#endif

bool __kmp_task_is_allowed(int gtid, const kmp_int32 is_constrained,
                                  const kmp_taskdata_t *tasknew,
                                  const kmp_taskdata_t *taskcurr);
#if HCLIB_OMP
extern "C" void hclib_interop_init() {
//  if (hclib_get_current_worker() == -2) {
  interop_var_initialize();
//  }
  if (interop_get_worker_id() != -2) {
//  if (hclib_get_current_worker() != 0)
    __kmp_get_global_thread_id_reg(); 
    __kmp_threads[__kmp_get_gtid()]->th.th_worker_obtained = 1;
    __kmp_threads[__kmp_get_gtid()]->th.th_cur_coreID = interop_get_worker_id();
  // This worker is uber for initial root thread 
    if (interop_get_worker_id() == 0) {
      // The worker with id 0 will be resumed if the first thread initiating OpenMP runtime is terminated
      interop_ext_comm *ptr = &(interop_thread_local(commStruct));
      pthread_mutex_lock(&(ptr->mutex));
      KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d  is suspended on worker [%d, %p]\n", __kmp_get_gtid(), interop_get_worker_id(),interop_get_worker_state()));
      ptr->worker_suspended = 1;
      pthread_cond_wait(&(ptr->cond), &(ptr->mutex));
      interop_steal_worker_state(0);
      ptr->worker_suspended = 0;
      KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d  is resumed on worker [%d, %p]\n", __kmp_get_gtid(), interop_get_worker_id(),interop_get_worker_state()));

      pthread_mutex_unlock(&(ptr->mutex));
    }
  }
}
// This is used when hclib runtime is strated first before OpenMP runtime
extern "C" void hclib_interop_init_master() {
//  if (hclib_get_current_worker() == -2) {
  // This worker is uber for initial root thread 
    __kmp_get_global_thread_id_reg(); 
}


#elif CHARM_OMP && CONV_OMP
static void 
__kmp_conv_init(int argc, char **argv) {
  (void)argc;
  (void)argv;

  RegisterOmpHdlrs();
  CmiInitCPUAffinity(argv);
  CmiInitMemAffinity(argv);
  CmiInitCPUTopology(argv);
  CmiNodeAllBarrier(); 
  interop_thread_local(inOpenMP)=0;
  //CmiCheckAffinity();
  /* Skip comm and master thread */ 
  if (interop_get_worker_id() != interop_get_num_workers() || interop_get_worker_id() != 0) {
    CsdScheduler(-1);
    ConverseExit();
  }
}
//char **argv;
//int argc;

static void* __kmp_launch_root_uber(void *data) {
  pthread_mutex_lock(&(__kmp_uber_mutex.m_mutex));
  pthread_cond_wait(&(__kmp_uber_cond.c_cond), &(__kmp_uber_mutex.m_mutex));
  pthread_mutex_unlock(&(__kmp_uber_mutex.m_mutex));
  /* Inherit CmiState of the rank 0 */
  interop_steal_worker_state(0);
  interop_thread_local(uberThreadStart) = 1;
  __kmp_get_global_thread_id_reg();
  CsdScheduler(-1);
  ConverseExit();
}

#endif
int __kmp_get_cpu_id(int gtid) {
  kmp_int32 success;
  kmp_int32 cur_val;
  do {
    cur_val = TCR_4(__kmp_core_id_idx);
    success = KMP_COMPARE_AND_STORE_REL32(&__kmp_core_id_idx, cur_val, (cur_val + 1)%num_workers);
    if (success && gtid !=0 && __kmp_core_id_list[cur_val] !=0)
        break;
  } while (1);
  return __kmp_core_id_list[cur_val];
}

void interop_var_initialize() {
#if HCLIB_OMP
/*  if (interop_get_current_worker()
  prevGtid = (int *) __kmp_allocate(sizeof(int) * interop_get_num_workers());
  masterRunning = (int*) __kmp_allocate(sizeof(int) * interop_get_num_workers());
  commStruct = (interop_ext_comm *) __kmp_allocate(sizeof(interop_ext_comm)*interop_get_num_workers());*/
//#pragma ivdep
  int tid = interop_get_worker_id();
  //Initial thread have this id number
  if (tid == -2) {
//    prevGtid =  new int[interop_get_num_workers()]{-2}; //(int *) __kmp_allocate(sizeof(int) * interop_get_num_workers());
    prevGtid = new std::vector<std::vector<int>>(interop_get_num_workers(),std::vector<int>(INIT_NEST_LEVEL, -2));
    masterRunning = new std::vector<std::vector<int>>(interop_get_num_workers(), std::vector<int>(INIT_NEST_LEVEL, 0));

/*    for (int i =0 ; i < interop_get_num_workers(); i++) {
      prevGtid[i] = new std::vector<int>(INIT_NEST_LEVEL, -2);
      masterRunning[i] = new std::vector<int>(INIT_NEST_LEVEL, 0);
    }*/
//    masterRunning = new int[interop_get_num_workers()]{0}; //(int*) __kmp_allocate(sizeof(int) * interop_get_num_workers());
    commStruct = new interop_ext_comm[interop_get_num_workers()](); //(interop_ext_comm *) __kmp_allocate(sizeof(interop_ext_comm)*interop_get_num_workers());
    interop_worker_init = new std::vector<AlignedInt>(interop_get_num_workers());// new int[interop_get_num_workers()]{0};
    inOpenMP = new std::vector<AlignedInt>(interop_get_num_workers());// new int[interop_get_num_workers()]{0};

    nest_level = new std::vector<AlignedInt>(interop_get_num_workers());  // int[interop_get_num_workers()]{0};
    tid = 0;
    atomic_thread_fence(std::memory_order_release);
  }
  
/*  interop_thread_local(prevGtid) = -2;
  interop_thread_local(masterRunning) = 0;
  interop_thread_local(commStruct).head = 0;
  interop_thread_local(commStruct).tail = 0;
  interop_thread_local(commStruct).extArrived = 0;
  interop_thread_local(commStruct).worker_suspended = 0;
  interop_thread_local(commStruct).numWaitingThread = 0;*/
  if (interop_get_worker_id () != 0) {
    pthread_cond_init(&(interop_thread_local_other(commStruct, tid).cond), NULL);
    pthread_mutex_init(&(interop_thread_local_other(commStruct, tid).mutex), NULL);
    interop_thread_local_other(commStruct,tid).extCond = (pthread_cond_t *) __kmp_allocate(sizeof(pthread_cond_t) * interop_get_num_workers());
    for (int j=0; j< interop_get_num_workers(); j++)
        pthread_cond_init(&(interop_thread_local_other(commStruct, tid).extCond[j]), NULL);
  }
/*    for (int i=0; i< interop_get_num_workers(); i++) {
      interop_thread_local_other(prevGtid, i) = -2;
      interop_thread_local_other(masterRunning, i) = 0;
      interop_thread_local_other(commStruct, i).head = 0;
      interop_thread_local_other(commStruct, i).tail = 0;
      interop_thread_local_other(commStruct, i).extArrived = 0;
      interop_thread_local_other(commStruct, i).worker_suspended = 0;
      interop_thread_local_other(commStruct, i).numWaitingThread = 0;
      pthread_cond_init(&(interop_thread_local_other(commStruct, i).cond), NULL);
      pthread_mutex_init(&(interop_thread_local_other(commStruct, i).mutex), NULL);
      interop_thread_local_other(commStruct, i).extCond = (pthread_cond_t *) __kmp_allocate(sizeof(pthread_cond_t) * interop_get_num_workers());
      for (int j=0; j< interop_get_num_workers(); j++)
          pthread_cond_init(&(interop_thread_local_other(commStruct,i).extCond[j]), NULL);
  }*/
#elif CHARM_OMP || CONV_OMP
  initGlobalVar();
#endif
}

void interop_initialize() {
  int num_ext_threads;
  interop_init_workers.store(0, std::memory_order_release);
#if HCLIB_OMP
//  int num_spmd_instances = getenv("OMP_NUM_THREADS");
  char const *deps[] = { "system" };
  // allocate arrays for thread local variables
  // Initialization will be done on each worker in 'hclib_interop_init'
/*  prevGtid = (int *) __kmp_allocate(sizeof(int) * interop_get_num_workers());
  masterRunning = (int*) __kmp_allocate(sizeof(int) * interop_get_num_workers());
  commStruct = (interop_ext_comm *) __kmp_allocate(sizeof(interop_ext_comm)*interop_get_num_workers());
*/
  hclib_init_external(deps, 1, 0, hclib_interop_init, 1);
#elif CHARM_OMP && CONV_OMP
  //char *argv1 = "openmp";
  ////char *argv2 = "+p8";
  /* To do list, better parsing for interop_omp_init is needed */
  argv = (char**)__kmp_allocate(sizeof(char*)*4);
  argv[0] = (char*) __kmp_allocate(sizeof(char) *8);
  argv[1] = (char*) __kmp_allocate(sizeof(char) *8);
  argv[2] = (char*) __kmp_allocate(sizeof(char) *14);
  argv[3] = NULL;
  argc = 3;
  //snprintf(argv[0],sizeof(char)*7, "openmp");
  /* number of cores, use OMP_NUM_THREADS */
  snprintf(argv[0], sizeof(char)*7, "+p%d", __kmp_dflt_team_nth);
  /* thread affinity for converse threads, use OMP_PLACE_THREADS */
  if (getenv("CONV_NUM_THREADS"))
    snprintf(argv[0], sizeof(char)*7, "+p%s", getenv("CONV_NUM_THREADS"));
  else
    snprintf(argv[0], sizeof(char)*7, "+p%d", __kmp_dflt_team_nth);

  if (getenv("CONV_PEMAP")) {
    snprintf(argv[1], sizeof(char)*8, "+pemap ");
    snprintf(argv[2], sizeof(char)*14, "%s", getenv("CONV_PEMAP"));
  }
  else {
    __kmp_free(argv[1]);
    __kmp_free(argv[2]);
    argv[1]=NULL;
    argv[2]=NULL;
    argc = 1;
  }
//  int myRank = sched_getcpu();
//  pthread_setspecific(__kmp_curPE, &myRank);
  ConverseInit(argc, argv, __kmp_conv_init, 1, 1);
#endif
  if (getenv("INTEROP_EXT_NUM_THREADS"))
    num_ext_threads = atoi(getenv("INTEROP_EXT_NUM_THREADS"));
  else
    num_ext_threads = 1;

  // initialize global / thread local variables
//  interop_var_initialize();
  TCW_4(__kmp_initial_root_exit, 0);
  /* create uber thread */
  pthread_cond_init(&__kmp_uber_cond.c_cond, NULL);
  pthread_cond_init(&__kmp_init_cond.c_cond,NULL);
  pthread_mutex_init(&__kmp_uber_mutex.m_mutex, NULL);
  pthread_mutex_init(&__kmp_init_mutex.m_mutex, NULL);

  //pthread_create(&__kmp_root_uber_thread, NULL, __kmp_launch_root_uber, NULL);
  num_workers = interop_get_num_workers();

  // initialize affinity of external threads  
  CPU_ZERO(&allCPU);
  for (int i = 0 ; i < get_nprocs(); i++)
    CPU_SET(i, &allCPU);
  std::vector<kmp_int32>__kmp_core_id_list_total; // = (kmp_int32*)__kmp_allocate(sizeof(kmp_int32) * num_workers);
  std::vector<kmp_int32>::iterator it;
  int div = num_workers/2;
  int k = 2;
  int prev = k;
  __kmp_core_id_list_total.resize(num_workers);
  __kmp_core_id_list_total[0] = 0;
  __kmp_core_id_list_total[1] = div;
  __kmp_core_id_idx = 0;
  
  for (int i =2; i < num_workers ; ) {
    div = div/2;
    if (div ==0) div =1;
    for (int j =i; j < i+k && j < num_workers; j++) {
      //printf("i,j,k,div,prev: %d, %d, %d, %d, %d\n", i,j,k,div,prev);
      __kmp_core_id_list_total[j] = __kmp_core_id_list_total[j-prev] + div;
    }
    i+=k;
    if (div !=1)
      k=k*2, prev=k;
    //k=k*2;
    //prev = k;
  }
  __kmp_printf("# ext threads: %d\n", num_ext_threads);
  for (int i=0; i< num_workers; i++)
    __kmp_printf("%d,", __kmp_core_id_list_total[i]);
  __kmp_core_id_list.resize(num_workers);
  it = __kmp_core_id_list_total.begin();
  __kmp_core_id_list.insert(__kmp_core_id_list.begin(), it, it+num_ext_threads);
  std::sort(__kmp_core_id_list.begin(), __kmp_core_id_list.begin()+num_ext_threads);
  
  __kmp_printf("core_id_list_sorted: ");
  for (int i=0; i< num_ext_threads; i++)
    __kmp_printf("%d, ", __kmp_core_id_list[i]);
  __kmp_printf("\n");
  __kmp_core_id_list.insert(__kmp_core_id_list.begin()+num_ext_threads, it+num_ext_threads, it+num_workers);  
  
  __kmp_printf("core_id_list: ");
  for (int i=0; i< num_workers; i++)
    __kmp_printf("%d,", __kmp_core_id_list[i]);

  __kmp_printf("\n");
  std::atomic_thread_fence(std::memory_order_release);
}

void interop_root_initialize(void *ptr, int gtid, int initial_thread) {
  kmp_info_t *root_thread = (kmp_info_t*)ptr;
  /* Only workers in interop runtimes initializes these values */
  if (interop_thread_local(interop_worker_init)!=1 && interop_get_worker_id() >= 0 || initial_thread) {
//#if HCLIB_OMP
//#elif CHARM_OMP
    interop_thread_local(prevGtid) = gtid; //, interop_get_worker_id()) = gtid;
#if CONV_OMP
    interop_thread_local(prevConvGtid) = gtid;
#endif
//    if (interop_init_workers.fetch_add(1, std::memory_order_acq_rel) == interop_get_num_workers()-1) {
//        interop_omp_init = 1;
  //      KMP_MB();
//    }
    interop_thread_local(interop_worker_init) = 1;    
//#endif
  }
  if (initial_thread) {
      root_thread->th.th_worker_obtained = 1;
      root_thread->th.th_cur_coreID = 0;
  }
  else {
      root_thread->th.th_worker_obtained = 0;
      root_thread->th.th_cur_coreID = -1;
  }
//#endif
}

void interop_post_initialize() { //int argc, char **argv) {
//#if HCLIB_OMP
//    interop_omp_init = 1;
#if CHARM_OMP && CONV_OMP
  __kmp_release_bootstrap_lock(&__kmp_initz_lock);
  CmiInitCPUAffinity(argv);
  CmiInitMemAffinity(argv);
  CmiInitCPUTopology(argv);
  CmiNodeAllBarrier();
  //CmiCheckAffinity();
  pthread_mutex_lock(&__kmp_init_mutex.m_mutex);
  KMP_MB();
  pthread_cond_broadcast(&__kmp_init_cond.c_cond);
  pthread_mutex_unlock(&__kmp_init_mutex.m_mutex);
//  std::atomic_thread_fence(std::memory_order_release);

  __kmp_acquire_bootstrap_lock(&__kmp_initz_lock);
#endif
}

//#if CONV_OMP
__attribute__((constructor)) void __kmp_replace_sched_yield(void) {
  *(void**)(&real_sched_yield) = dlsym(RTLD_NEXT, "sched_yield");
}
//#endif

extern "C" int sched_yield(void) {
  if (real_sched_yield == NULL)
    *(void**)(&real_sched_yield) = dlsym(RTLD_NEXT, "sched_yield");

  if (interop_get_worker_id() < 0 || interop_get_worker_id() >= interop_get_num_workers()) {
    return real_sched_yield();
  }

  if (interop_worker_init && interop_thread_local(interop_worker_init) && inOpenMP && interop_thread_local(inOpenMP)) {
    //printf("sched_yield_replacing\n");
    int th_gtid = __kmp_gtid_get_specific(); 
    if (KMP_MASTER_GTID(th_gtid)) {
     interop_thread_local(masterRunning) = 1;
     void *msg;
// Schedule a task     
#if HCLIB_OMP
     hclib_yield(hclib_get_closest_locale());
#elif CHARM_OMP || CONV_OMP
#if CONV_OMP
      msg = CsdNextMessageExt();
      if (msg)
        CmiHandleMessage(msg);
      else
        StealTask();
#else
      msg = TaskQueuePop((TaskQueue)interop_thread_local(CsdTaskQueue));
      if (msg) {
        CmiHandleMessage(msg);
      }
      else {
        void *msg = CmiSuspendedTaskPop();
        if (msg) 
          CmiHandleMessage(msg);
        else 
          StealTask();
     }
#endif
#endif
     interop_thread_local(inOpenMP)=1;
     interop_thread_local(masterRunning) = 0;
     real_sched_yield();
    }
    else {
//      interop_set_global_variable(th_gtid, 0, 0);
/*      if (interop_thread_local(prevGtid) >=0) {
#if KMP_TDATA_GTID
        __kmp_gtid = interop_thread_local(prevGtid);
#endif
        __kmp_gtid_set_specific(interop_thread_local(prevGtid));
      }*/
#if CHARM_OMP || CONV_OMP
      CthSetStrategyWorkStealing(__kmp_threads[th_gtid]->th.th_info.ds.ds_thread);
#endif
//      inOpenMP=0;
//      std::atomic_thread_fence(std::memory_order_release);
#if HCLIB_OMP
      real_sched_yield();
//      hclib_yield_ult();
//      hclib_schedule_suspend(hclib_get_closest_locale(), 1);
#elif CHARM_OMP || CONV_OMP
      CthYield();
#endif
//      interop_set_global_variable(th_gtid, 1, 0);
//      inOpenMP=1;
      //StealTask();
/*
      __kmp_gtid_set_specific(th_gtid);
#if KMP_TDATA_GTID
      __kmp_gtid = th_gtid;
#endif
*/
           //inOpenMP=1;
    /*  else {
        inOpenMP=0;
        CthYield();
        inOpenMP=1;
      }*/
    }
  }
  else {
    //printf("Original\n");
    real_sched_yield();
  }

  return 0;
}

extern "C" unsigned short interop_get_random() {
    int gtid = __kmp_get_gtid(); 
    KMP_DEBUG_ASSERT(gtid >= 0);
    return __kmp_get_random(__kmp_threads[gtid]);
}

extern "C" void interop_check_worker_suspended(int worker_id) {

    std::atomic_thread_fence(std::memory_order_acquire);
    interop_ext_comm *ptr = &(interop_thread_local_other(commStruct, worker_id));
    if (ptr->extArrived || (worker_id == 0 && !first_worker_early_exit)) 
       return;
//    if (ptr->worker_suspended) {
        pthread_mutex_lock(&(ptr->mutex));
        if (ptr->worker_suspended)
            pthread_cond_signal(&(ptr->cond));
        pthread_mutex_unlock(&(ptr->mutex));

/*        do {
            asm("");
        } while (ptr->worker_suspended);*/
//    }
}

extern "C" void interop_check_ext_thread(volatile int *cur_gang_cnt) {
    // This function is called on workers of the underlying threading runtime
  if (interop_worker_init && interop_thread_local(interop_worker_init)) {
      interop_ext_comm *ptr = &(interop_thread_local(commStruct));
      std::atomic_thread_fence(std::memory_order_acquire);
      if (ptr->extArrived || cur_gang_cnt && (*cur_gang_cnt)==0) {
          kmp_int32 gtid = __kmp_gtid_get_specific(); 
          kmp_info_t *master_th = __kmp_threads[gtid];
          pthread_mutex_lock(&(ptr->mutex));
          ptr->worker_suspended = 1;
          //std::atomic_thread_fence(std::memory_order_acq_rel);
          //KMP_MB();
          while (((ptr->extArrived) || cur_gang_cnt && (*cur_gang_cnt)==0)) {
              if (!interop_thread_local(interop_worker_init))
                  break;
//              ptr->numWaitingThread++;
              KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d  will be suspended on worker [%d, %p]\n", gtid, interop_get_worker_id(),interop_get_worker_state()));
              pthread_cond_wait(&(ptr->cond), &(ptr->mutex));
              atomic_thread_fence(std::memory_order_acquire);
/*              if (*cur_gang_cnt == 0 || ptr->extArrived)
                  KMP_ASSERT(0);*/
              KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d  is resumed on worker [%d, %p]\n", gtid, interop_get_worker_id(),interop_get_worker_state()));
          }
          ptr->worker_suspended = 0;
          pthread_mutex_unlock(&(ptr->mutex));
          return;
      } 
//      sched_yield();
  }
}

void interop_suspend_worker(kmp_int32 gtid, kmp_team_t *parent_team) {
  /* pin this external thread to the current core 
   * The algorithm how the external threads are mapped will be changed
   * The worker which was running on this core is suspended and the worker state info in the underlying runtime
   * will be used by this external thread for scheduling user-level threads on OpenMP
   * */
  //int gtid = master_th->th.th_info.ds.ds_gtid;
  kmp_info_t* master_th = __kmp_threads[gtid]; 
  if ( !master_th->th.th_worker_obtained && parent_team->t.t_level == 0 ) { //interop_get_worker_id() == -2 &&
    int worker_found = 0;
    int affinity_set = 0;
    if (master_th->th.th_cur_coreID == -1) {
      master_th->th.th_cur_coreID = __kmp_get_cpu_id(gtid); //sched_getcpu();
    } else {
        affinity_set = 1;
    }
#if 0 //KMP_DEBUG
    __kmp_printf("gtid: %d, core: %d\n",gtid, master_th->th.th_cur_coreID);
#endif
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    interop_ext_comm *ptr = &(interop_thread_local_other(commStruct, master_th->th.th_cur_coreID));
    pthread_mutex_lock(&(ptr->mutex));

    if (!ptr->extArrived) {
      ptr->extArrived = 1;
      KMP_MB();
      pthread_mutex_unlock(&(ptr->mutex));
      worker_found = 1;
    }
    /* signal that the external thread arrived */
    else { /* (interop_thread_local(commStruct).extArrived)*/
      KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d [%d] this converse thread is already being used for another master thread\n",gtid, master_th->th.th_cur_coreID));
      pthread_mutex_unlock(&(ptr->mutex));

      /* search over not used converse thread */ 
      for (int i = 1; i < num_workers ; i++) {
        if (i == master_th->th.th_cur_coreID) continue;
        ptr = &(interop_thread_local_other(commStruct, i));
        pthread_mutex_lock(&(ptr->mutex));
            if (!ptr->extArrived) {
              ptr->extArrived = 1;
              master_th->th.th_cur_coreID = i;
              worker_found = 1;
              pthread_mutex_unlock(&(ptr->mutex));
              break;
            }
            else {
              pthread_mutex_unlock(&(ptr->mutex));
              continue;
            }
      }
      /* all converse threads are busy
       * This thread will wait for the external thread running on the same core to finish
       * */
      if (worker_found == 0) {
        ptr = &(interop_thread_local_other(commStruct, master_th->th.th_cur_coreID));
        pthread_mutex_lock(&(ptr->mutex));
        ptr->numWaitingThread++;
        int curSlot = ptr->tail;
        ptr->tail = (ptr->tail+1) % interop_get_num_workers();
        while (ptr->extArrived){
          KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d will be suspended at %p [%d, %p]\n", gtid, &ptr->extCond[curSlot], master_th->th.th_cur_coreID,interop_get_worker_state()));
          pthread_cond_wait(&ptr->extCond[curSlot], &(ptr->mutex));
        } // while(ptr->extArrived);
        ptr->numWaitingThread--;
        ptr->head = (ptr->head+1) % interop_get_num_workers();
        ptr->extArrived = 1;
        KMP_MB();
        pthread_mutex_unlock(&(ptr->mutex));
        worker_found = 1;
      }
    }  

    interop_steal_worker_state(master_th->th.th_cur_coreID);
    KA_TRACE(1, ("__kmp_suspend_conv_thread: T#%d will suspend conv thread[%d, %p], master_th->th.th_worker_obtained: %d\n", gtid, interop_get_worker_id(),interop_get_worker_state(), master_th->th.th_worker_obtained));

    if (!affinity_set) {
        CPU_SET(master_th->th.th_cur_coreID, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    }
    if (!TCR_4(__kmp_initial_root_exit)) { //master_th->th.th_cur_coreID != 0 || 
      /* when this parallel region is nested, the converse thread is already suspended */ 
      while (!interop_thread_local(commStruct).worker_suspended && parent_team->t.t_level == 0) {
        KMP_DEBUG_ASSERT(interop_thread_local(commStruct).extArrived == 1);
        real_sched_yield();
        //usleep(1);
        std::atomic_thread_fence(std::memory_order_acquire);
      }
    }

   /* set prevGtid with the gtid of this external thread */ 
    master_th->th.th_worker_obtained = 1;
    master_th->th.th_ext_thread = 1;
    interop_thread_local(prevGtid)=__kmp_gtid_get_specific();
#ifdef KMP_TDATA_GTID
    interop_thread_local(prevGtid)=__kmp_gtid;
#endif
    KMP_MB();
  }
}

void interop_var_finalize() {
#if HCLIB_OMP
/*  for (int i =0; i<interop_get_num_workers();i++) {
    delete prevGtid[i];
    delete masterRunning[i];
  }*/
  delete prevGtid;
  delete masterRunning;
  delete interop_worker_init;
  delete inOpenMP;
  delete nest_level;
//  __kmp_free(prevGtid); // = (int *) aligned_alloc(getpagesize(), sizeof(int) * interop_get_num_workers());
//  __kmp_free(masterRunning); // = (int*) aligned_alloc(getpagesize(), sizeof(int) * interop_get_num_workers());
#endif
}


void interop_wake_up_worker(int rank) {
  interop_ext_comm *ptr = &(interop_thread_local_other(commStruct, rank));
  ptr->extArrived = 0;
  pthread_mutex_lock(&(ptr->mutex));
  if (ptr->worker_suspended)
      pthread_cond_signal(&(ptr->cond));
  pthread_mutex_unlock(&(ptr->mutex));
}

void interop_release_worker() { 
  kmp_info_t *master_th = __kmp_threads[__kmp_get_gtid()];
  kmp_team_t *team  = master_th->th.th_team;
  if (team->t.t_level == 0 && master_th->th.th_ext_thread) {
      master_th->th.th_worker_obtained = 0;
      interop_wake_up_worker(interop_get_worker_id());
  }
}

void interop_signal_finalize() {
  //interop_omp_init=0;
//  KMP_MB();
#if HCLIB_OMP
#elif CHARM_OMP && CONV_OMP
  CsvAccess(ConvOpenMPExit) = 1;
  KMP_MB();
#endif
//#pragma unroll
  for (int i = 0; i < num_workers; i++) {
    interop_thread_local_other(interop_worker_init, i) = 0;
    interop_wake_up_worker(i);
  }
  KMP_MB();

/*  for (int i = 0; i < num_workers; i++) {
//    if (interop_thread_local_other(commStruct,i).extArrived == 1) {
      interop_wake_up_worker(i);
//    }
  }*/
}

void interop_finalize() {
  TCW_4(__kmp_initial_root_exit, 1);
#if HCLIB_OMP
  // If this thread is not hclib worker, then one of hclib workers run as the first worker.
  // The worker should be suspended before we start termination of hclib runtime
//  if (interop_get_worker_id() != 0) {
    interop_ext_comm *ptr = &(interop_thread_local_other(commStruct,0));
    hclib_join_tid(0);
    if (interop_get_worker_id() !=0)
        interop_steal_worker_state(0);
/*    if (!ptr->worker_suspended) {
    }*/
//  }
  hclib_finalize_external(0);
  interop_var_finalize();
  for (int i =0; i< interop_get_num_workers(); i++) {
      __kmp_free(interop_thread_local_other(commStruct,i).extCond);
  }
  delete commStruct;
  //__kmp_free(commStruct);
#elif CHARM_OMP && CONV_OMP
  ConverseExit();
#endif
  /* if (!interop_thread_local(uberThreadStart)) {
    pthread_mutex_lock(&__kmp_uber_mutex.m_mutex);
    pthread_cond_signal(&(__kmp_uber_cond.c_cond));
    pthread_mutex_unlock(&__kmp_uber_mutex.m_mutex);
    pthread_join(__kmp_root_uber_thread, NULL);
  }*/
  //CkExit();
//  CmiNodeAllBarrier();
}

void interop_thread_finalize(int gtid, int prevCoreID, int gtid_req) {
//#if CONV_OMP
 // if (gtid == 0) {
//    TCW_4(__kmp_initial_root_exit, 1);
    /* wake up uber thread */
//    pthread_mutex_lock(&__kmp_uber_mutex.m_mutex);

//    pthread_cond_signal(&(__kmp_uber_cond.c_cond));
//    pthread_mutex_unlock(&__kmp_uber_mutex.m_mutex);
//  }
//  else {
//    if (gtid_req <0) {
      interop_ext_comm *ptr = &(interop_thread_local_other(commStruct, prevCoreID)); 
      pthread_mutex_lock(&(ptr->mutex));
      ptr->extArrived = 0;
      if (ptr->numWaitingThread > 0) {
        pthread_cond_signal(&ptr->extCond[ptr->head]);
        ptr->numWaitingThread--;
        ptr->head = (ptr->head+1) %interop_get_num_workers();
      //, &ptr->mutex);
      }
/*      else if (ptr->numWaitingThread == 0) {
        ptr->extArrived = 0;
      }*/
      else if (ptr->worker_suspended) {
        pthread_cond_signal(&ptr->cond);
      }
      if (interop_get_worker_id() == 0 && !first_worker_early_exit)
          first_worker_early_exit = 1;
      pthread_mutex_unlock(&(ptr->mutex));
//    }
//  }
//#endif
}

void interop_master_wait(kmp_info_t *th, volatile kmp_int32 *barrier_count, kmp_int32 reset_val, int sched_task) {

  interop_thread_local(masterRunning) = 1;
  void *msg = NULL;
  atomic_thread_fence(std::memory_order_acquire);
  while (*barrier_count > 0) {
    if (sched_task) {
#if HCLIB_OMP
      // If this worker is not involved in first gang, then it should schedule ULTs.
      hclib_yield(hclib_get_closest_locale());
#elif CHARM_OMP
#if CONV_OMP
      msg = CsdNextMessageExt();
      if (msg)
        CmiHandleMessage(msg);
      else
        StealTask();
#else
      msg = TaskQueuePop((TaskQueue)interop_thread_local(CsdTaskQueue));
      if (msg) {
        CmiHandleMessage(msg);
      }
      else {
        msg = CmiSuspendedTaskPop();
        if (msg) 
          CmiHandleMessage(msg);
        else
          StealTask();
      }
#endif
#endif
    } else {
      real_sched_yield();
    }
    atomic_thread_fence(std::memory_order_acquire);
  }
  KMP_ASSERT(*barrier_count == 0);
  if (reset_val >= 0)
    TCW_4(*barrier_count, reset_val);
  interop_thread_local(masterRunning) = 0;
}
#if INTEROP_OMP
extern void __kmp_invoke_task(kmp_int32 gtid, kmp_task_t *task,
                              kmp_taskdata_t *current_task);
#endif
// This functions is used in the interop runtime for openmp tasks
void interop_execute_task(void *arg) {
  kmp_task_t *task = (kmp_task_t*)(arg);
  kmp_taskdata_t *current_task = __kmp_threads[__kmp_get_gtid()]->th.th_current_task;
  // call __kmp_invoke_task on interop workers
  __kmp_invoke_task(__kmp_get_gtid(), task, current_task);
}

void* interop_create_task(void (*func)(void*), kmp_task_t *task) {
  return hclib_create_task(func, task);
}

void interop_schedule_task(void *interop_task) {
  KMP_ASSERT(interop_task);
  hclib_schedule_ext_task((hclib_task_t*)interop_task);
  return;
}


extern "C" bool interop_task_is_allowed(hclib_task_t *task) {
  kmp_int32 gtid = __kmp_get_gtid();
  kmp_info_t* thread = __kmp_threads[gtid];
//  hclib_task_t *cur_task = reinterpret_cast<hclib_task_t*>(task); 
  kmp_task_t *omp_task = (kmp_task_t*)(task->args);
  if(!__kmp_task_is_allowed(gtid, __kmp_task_stealing_constraint, KMP_TASK_TO_TASKDATA(omp_task), thread->th.th_current_task))
    return false;
  return true;
}

void* interop_pop_task(int gtid, kmp_int32 is_constrained,const kmp_taskdata_t *taskcurr ) {
  hclib_task_t *task = (hclib_task_t*) hclib_pop_ext_task();
  kmp_task_t *omp_task = NULL; 
  if (task) {
    omp_task = (kmp_task_t*)(task->args);
/*    if (!__kmp_task_is_allowed(gtid, is_constrained, KMP_TASK_TO_TASKDATA(omp_task), taskcurr)) {
        interop_schedule_task(task);
        omp_task = NULL;
    }*/
  }
  return omp_task;
}

void* interop_steal_task(int gtid, kmp_int32 is_constrained, const kmp_taskdata_t *taskcurr) {
  hclib_task_t *task =NULL;
  kmp_task_t *omp_task = NULL; 

  task = (hclib_task_t*) hclib_steal_ext_task();
  if (task) {
    omp_task = (kmp_task_t*)(task->args);
    KMP_ASSERT(omp_task->interop_task);
/*    if (!__kmp_task_is_allowed(gtid, is_constrained, KMP_TASK_TO_TASKDATA(omp_task), taskcurr)) {
        interop_schedule_task(task);
        omp_task = NULL;
    }*/
  }
  return omp_task;
}
void interop_execute_gang_task(int full) {
    hclib_execute_gang_task(full);
}


