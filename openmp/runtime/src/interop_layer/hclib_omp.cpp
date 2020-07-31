#include "hclib_omp.h"
std::vector<int> **prevGtid;
std::vector<int> **masterRunning;
int *nest_level;
interop_ext_comm *commStruct;
int *interop_worker_init;
