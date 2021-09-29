#ifndef HCLIB_OMP_H
#define HCLIB_OMP_H
#include "hclib.h"
#include "hclib-rt.h"
#include "interop_ext_comm.h"
#include <vector>
#define INIT_NEST_LEVEL 10
#define CACHE_LINE_SIZE_ 64
extern std::vector<std::vector<int>> *prevGtid;
extern std::vector<std::vector<int>> *masterRunning;
extern interop_ext_comm *commStruct;

struct AlignedInt {
  alignas(CACHE_LINE_SIZE_) int val=0;
};

extern std::vector<AlignedInt> *nest_level;
extern std::vector<AlignedInt> *interop_worker_init;
extern std::vector<AlignedInt> *inOpenMP;
#endif
