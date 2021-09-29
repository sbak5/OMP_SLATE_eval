#include "hclib_omp.h"
std::vector<std::vector<int>> *prevGtid;
std::vector<std::vector<int>> *masterRunning;


interop_ext_comm *commStruct;

std::vector<AlignedInt> *nest_level;
std::vector<AlignedInt> *interop_worker_init;
std::vector<AlignedInt> *inOpenMP;
