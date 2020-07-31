#if !CONV_OMP
#include "OmpCharm.decl.h"
#endif
#include "ompcharm.h"

extern void* __kmp_launch_worker(void *);
CpvDeclare(int, prevGtid);
CpvDeclare(unsigned int, localRatio);
CpvDeclare(unsigned int*, localRatioArray);
CpvDeclare(unsigned int, ratioIdx);
CpvDeclare(bool, ratioInit);
CpvDeclare(unsigned int, ratioSum);
CpvDeclare(int, curNumThreads);
CpvDeclare(int, masterRunning);
#if CONV_OMP
CpvDeclare(int, prevConvGtid);
CpvDeclare(int, uberThreadStart);
CpvDeclare(int, numLevel);
int numConvThreads;
#endif
extern int __kmp_get_global_thread_id_reg();
extern "C" void initGlobalVar() {
  CpvInitialize(int, prevGtid);
#if CONV_OMP
  CpvInitialize(int, prevConvGtid);
  CpvInitialize(int, uberThreadStart);
  CpvInitialize(int, numLevel);
#endif
  CpvInitialize(unsigned int, localRatio);
  CpvInitialize(unsigned int, ratioIdx);
  CpvInitialize(unsigned int, ratioSum);
  CpvInitialize(bool, ratioInit);
  CpvInitialize(unsigned int*, localRatioArray);
  CpvInitialize(int, curNumThreads);
  CpvInitialize(int, masterRunning);
  CpvAccess(localRatioArray) = (unsigned int*) __kmp_allocate(sizeof(unsigned int) * windowSize);
  memset(CpvAccess(localRatioArray), 0, sizeof (unsigned int) * windowSize);
  CpvAccess(localRatio) = 0;
  CpvAccess(ratioIdx) = 0;
  CpvAccess(ratioSum) = 0;
  CpvAccess(ratioInit) = false;
  CpvAccess(prevGtid) = -2;
#if CONV_OMP
  CpvAccess(prevConvGtid) = -2;
  CpvAccess(uberThreadStart) = 0;
  CpvAccess(numLevel) = 0;
#endif
  CpvAccess(masterRunning) = 0;
}

extern "C" void RegisterOmpHdlrs() {
  initGlobalVar();
  __kmp_get_global_thread_id_reg();
}

extern "C" int CmiGetCurKnownOmpThreads() {
  return CpvAccess(curNumThreads);
}

#if !CONV_OMP
#include "OmpCharm.def.h"
#endif
