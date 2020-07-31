#ifndef HCLIB_OMP_H
#define HCLIB_OMP_H
#include "hclib.h"
#include "hclib-rt.h"
#include "interop_ext_comm.h"
#include <vector>
#define INIT_NEST_LEVEL 10
extern std::vector<int> **prevGtid;
extern std::vector<int> **masterRunning;
extern interop_ext_comm *commStruct;
extern int *nest_level;
extern int *interop_worker_init;
#endif
