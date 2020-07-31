#ifndef __INTEROP_EXT_COMM_H__
#define __INTEROP_EXT_COMM_H__
#include <pthread.h>
typedef struct {
  pthread_cond_t cond;
/*  CmiExtCond *head;
  CmiExtCond *tail;
  CmiExtCond *waitList;*/
  pthread_cond_t *extCond=NULL;
  pthread_mutex_t mutex;
  int numWaitingThread=0;
  int head=0;
  int tail=0;
  int extArrived=0;
  int worker_suspended=0;
  //interop_ext_comm(); // : extCond(nullptr), numWaitingThread(0), head(0), tail(0), extArrived(0), worker_suspended(0) {}
} interop_ext_comm;
#endif
