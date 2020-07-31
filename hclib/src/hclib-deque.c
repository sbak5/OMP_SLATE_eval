/* Copyright (c) 2015, Rice University

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
3.  Neither the name of Rice University
     nor the names of its contributors may be used to endorse or
     promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

/*
 * hclib-deque.cpp
 *
 *      Author: Vivek Kumar (vivekk@rice.edu)
 *      Acknowledgments: https://wiki.rice.edu/confluence/display/HABANERO/People
 */

#include "hclib-internal.h"
#include "hclib-atomics.h"
#if INTEROP_OMP
extern bool interop_task_is_allowed(hclib_task_t *task); 
extern void interop_execute_task(void *arg);

//int is_eligible_to_schedule_gang(hclib_task_t *task);
int is_eligible_to_schedule_gang(hclib_task_t *task) {
    if (!task) 
        return 0;
/*    else if (task->assigned_workers_ptr && !task->assigned_workers_ptr->steal_ready)
        return 0;*/

    hclib_worker_state * ws = CURRENT_WS_INTERNAL;  
    // If the task gang id is smaller than the worker's current gang_id or task's are in higher nested level
    // bottom-most parallel regions have higher priority
    int result = 0;
    if (ws->gang_monotonic_id[ws->internal_nest_level] == -1)
        result = 1;
    else if (task->gang_nested_level > ws->gang_nested_level[ws->internal_nest_level])
        result = 1;
    else if (task->gang_nested_level == ws->gang_nested_level[ws->internal_nest_level]) {
        if (task->gang_id < ws->gang_monotonic_id[ws->internal_nest_level])
            result = 1;
        else if (!task->persistent)
            result = 1;
/*        else if (task->gang_id < ws->last_pushed_gang_id && !deque_size(&(ws->paths->steal_path->locales[0]->gang_deq))) // task->gang_id >= gang_monotonic_id but it's smaller than lastly pushed_gang_id for this worker
            result = 1;*/
    }
    // If this steal happens on remote workers, then we need to first check if tasks for this gang are being pushed
    // If so, a task may be scheduled on this worker. 
    // To prevent unnecessary stealing, we postpone the stealing of a task if the gang of the current task is being pushed to workers. -> After all workers are pushed, we can steal the tasks from the gang. 

//    atomic_thread_fence(memory_order_acquire);
//    hclib_internal_deque_t *local_deq = ws->paths->steal_path->locales[0]->gang_deq;

    if (task->assigned_workers_ptr && !task->assigned_workers_ptr->steal_ready)
        result = 0;
/*    else {
          hclib_task_t *cur_local_task = local_deq->data[local_deq->head];
          if (deque_size(local_deq) && task != cur_local_task) { // Steal gang task from other threads
          //if (cur_local_task && cur_local_task->gang_id < task->gang_id)
  //            result = 0;
      }*
    }*/
/*    if (!task->assigned_workers_ptr->steal_ready)
        result = 0;*/
//    if (deque_size(&(ws->paths->steal_path->locales[0]->gang_deq))) // checks if there's any gang in thread local-queue
//        result = 0;
    return result;
//    return task->gang_id <= ws->gang_monotonic_id || task->gang_nested_level > ws->gang_nested_level; 
}
#endif

void deque_init(hclib_internal_deque_t *deq, void *init_value) {
    deq->head = 0;
    deq->tail = 0;
}

/*
 * push an entry onto the tail of the deque
 */
int deque_push(hclib_internal_deque_t *deq, void *entry) {
    atomic_thread_fence(memory_order_acq_rel);
//    hc_mfence();
    int size = deq->tail - deq->head;
    if (size == INIT_DEQUE_CAPACITY) { /* deque looks full */
        /* may not grow the deque if some interleaving steal occur */
        return 0;
    }
//    const int n = (deq->tail) % INIT_DEQUE_CAPACITY;
    deq->data[ (deq->tail) % INIT_DEQUE_CAPACITY] = (hclib_task_t *) entry;

    // Required to guarantee ordering of setting data[n] with incrementing tail.
    atomic_thread_fence(memory_order_acquire);
//    hc_mfence();
    atomic_fetch_add_explicit(&deq->tail, 1, memory_order_release);
//    deq->tail++;
//    atomic_thread_fence(memory_order_release);
//    deq->tail++;
//    atomic_fetch_add_explicit(&deq->tail, 1, memory_order_acq_rel);
    return 1;
}

int deque_push_gang(hclib_internal_deque_t *deq, void *entry) {
    atomic_thread_fence(memory_order_acquire);
//    hc_mfence();
    int head = deq->head;
    int tail = deq->tail;
    int size = tail - head;
    if (size == INIT_DEQUE_CAPACITY) {
      return 0;
    }
    do {
      //    const int n = (deq->tail) % INIT_DEQUE_CAPACITY;
      if (atomic_compare_exchange_strong_explicit(&deq->tail, &tail, tail+1, memory_order_release, memory_order_acquire)) {
        deq->data[ (tail) % INIT_DEQUE_CAPACITY] = (hclib_task_t *) entry;
        break;
      } 
    } while (tail - head <INIT_DEQUE_CAPACITY);
    // Required to guarantee ordering of setting data[n] with incrementing tail.
 //   atomic_thread_fence(memory_order_acq_rel);
//    hc_mfence();
//    deq->tail++;
//    atomic_thread_fence(memory_order_release);
//    deq->tail++;
//    atomic_fetch_add_explicit(&deq->tail, 1, memory_order_acq_rel);
    return 1;
}



void deque_destroy(hclib_internal_deque_t *deq) {
    free(deq);
}

/*
 * The steal protocol. Returns the number of tasks stolen, up to
 * STEAL_CHUNK_SIZE. stolen must have enough space to store up to
 * STEAL_CHUNK_SIZE task pointers.
 */
int deque_steal(hclib_internal_deque_t *deq, void **stolen) {
    /* Cannot read deq->data[head] here
     * Can happen that head=tail=0, then the owner of the deq pushes
     * a new task when stealer is here in the code, resulting in head=0, tail=1
     * All other checks down-below will be valid, but the old value of the buffer head
     * would be returned by the steal rather than the new pushed value.
     */

    int nstolen = 0;

    int success;

//    atomic_thread_fence(memory_order_acquire);
//    atomic_thread_fence(memory_order_acquire);    

    atomic_thread_fence(memory_order_acquire);
    int head = deq->head;
//    hc_mfence();
    int tail = deq->tail;
    do {
     // int head = deq->head;
//        hc_mfence();
//        atomic_thread_fence(memory_order_acquire);

        if ((tail - head) <= 0) {
            success = 0;
        } else {
            hclib_task_t *t = (hclib_task_t *) deq->data[head % INIT_DEQUE_CAPACITY];
            if (!t)
              HASSERT(0);
            if (t->_fp == interop_execute_task && !interop_task_is_allowed(t)) { 
                success=0; 
                break;
            }
            /* compete with other thieves and possibly the owner (if the size == 1) */
//            const int old = atomic_exchange_strong_explicit(&deq->head, &head, head+1, memory_order_relase, memory_order_acquire); //hc_cas(&deq->head, head, head + 1);
            if (atomic_compare_exchange_strong_explicit(&deq->head, &head, head+1, memory_order_release, memory_order_acquire)) { //  old == head) {
                success = 1;
                stolen[nstolen++] = t;
            }  else {
                success = 0;
                head = deq->head;
                tail = deq->tail;
            }
        }
    } while (success && nstolen < STEAL_CHUNK_SIZE);

    return nstolen;
}

#if INTEROP_OMP
int deque_steal_gang(hclib_internal_deque_t *deq, void **stolen) {
    /* Cannot read deq->data[head] here
     * Can happen that head=tail=0, then the owner of the deq pushes
     * a new task when stealer is here in the code, resulting in head=0, tail=1
     * All other checks down-below will be valid, but the old value of the buffer head
     * would be returned by the steal rather than the new pushed value.
     */

    int nstolen = 0;

    atomic_thread_fence(memory_order_acquire);
    int head = deq->head;
//    hc_mfence(); 
    int tail = deq->tail;
    int success;
    do {
//        atomic_thread_fence(memory_order_acquire);
        if ((tail - head) <= 0) {
            success = 0;
        } else {
            hclib_task_t *t = (hclib_task_t *) deq->data[head % INIT_DEQUE_CAPACITY];
            if (!t)
                HASSERT(0);
            if (!is_eligible_to_schedule_gang(t)) {
                success  = 0;
                break;
            } 
            
                /* compete with other thieves and possibly the owner (if the size == 1) */
//                const int old = hc_cas(&deq->head, head, head + 1);
            if (atomic_compare_exchange_strong_explicit(&deq->head, &head, head+1, memory_order_release, memory_order_acquire)) { //  old == head) {
                success = 1;
                stolen[nstolen++] = t;
            } else {
                success = 0;
                head = deq->head;
                tail = deq->tail;
            }
        }
    } while (success && nstolen < STEAL_CHUNK_SIZE); // Only single chunk size is available  && nstolen < STEAL_CHUNK_SIZE);

    return nstolen;
}
#endif

/*
 * pop the task out of the deque from the tail
 */
hclib_task_t *deque_pop(hclib_internal_deque_t *deq) {

//    hc_mfence();
    atomic_thread_fence(memory_order_acquire);
    int tail = deq->tail;
    tail--;
    deq->tail = tail;
    hc_mfence();
//    atomic_thread_fence(memory_order_acquire);
    int head = deq->head;

    int size = tail - head;
    if (size < 0) {
        deq->tail = deq->head;
        atomic_thread_fence(memory_order_release);
        return NULL;
    }
    hclib_task_t *t = (hclib_task_t *) deq->data[tail % INIT_DEQUE_CAPACITY];

    if (size > 0) {
        return t;
    }

    /* now size == 1, I need to compete with the thieves */
//    const int old =  hc_cas(&deq->head, head, head + 1);
    if (!atomic_compare_exchange_strong_explicit(&deq->head, &head, head+1, memory_order_release, memory_order_acquire)) { // old != head) {
        t = NULL;
    }

    /* now the deque is empty */
    deq->tail = deq->head;
    atomic_thread_fence(memory_order_release);
    return t;
}

unsigned deque_size(hclib_internal_deque_t *deq) {
    atomic_thread_fence(memory_order_acquire);
    const int size = deq->tail - deq->head;
    if (size <= 0) return 0;
    else return (unsigned)size;
}
