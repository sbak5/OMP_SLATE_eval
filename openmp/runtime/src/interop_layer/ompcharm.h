#ifndef OMP_CHARM_H
#define OMP_CHARM_H
#include "converse.h"
#include "conv-config.h"
#include "kmp.h"
#if CONV_OMP
//extern "C" int ConvOpenMP;
//extern "C" int numConvThreads;
//extern "C" pthread_key_t Cmi_state_key;
//extern "C" CmiState Cmi_state_vector;
extern "C" void CmiStealState(int rank);
extern "C" void *CmiGetStateExt();

CpvExtern(interop_ext_comm, commStruct); 
#endif

CpvExtern(unsigned int, localRatio);
CpvExtern(unsigned int*, localRatioArray);
CpvExtern(unsigned int, ratioIdx);
CpvExtern(bool, ratioInit);
CpvExtern(unsigned int, ratioSum);
CpvExtern(int, prevGtid);
#if CONV_OMP
CpvExtern(int, prevConvGtid);
CpvExtern(int, uberThreadStart);
CpvExtern(int, numLevel);
CsvExtern(CmiNodeLock, CsdNodeQueueLock);
#endif
//CsvExtern(unsigned int, idleThreadsCnt);
CsvExtern(int, ConvOpenMPExit);
CpvExtern(int, curNumThreads);
//CpvExtern(int, OmpHandlerIdx);

/* This variable is turned on when master thread executes ULTs in its taskqueue */
CpvExtern(int, masterRunning);

extern "C" void StealTask();
extern "C" void RegisterOmpHdlrs();
extern "C" void initGlobalVar();
#define CharmOMPDebug(...) // CmiPrintf(__VA_ARGS__)
// the intial ratio of OpenMP tasks in local list and work-stealing taskqueue
#define INITIAL_RATIO 2
#define windowSize 16
#endif
