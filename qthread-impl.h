/*
 * file:        qthread-impl.h
 * description: implementation-specific data structures for qthreads
 *              mutexes and condition variables.
 * Peter Desnoyers, Northeastern CCIS 2014
 * $Id:$
 */
#ifndef __QTHREAD_IMPL_H__
#define __QTHREAD_IMPL_H__

/* removed:   QTHREAD_MUTEX_INITIALIZER
              QTHREAD_COND_INITIALIZER 
   philosopher.c and websvr/listener.c modified to use explicit init
   if you want to add them back in, feel free.
*/

/* This file contains private definitions used only in qthread.c.
 */

/* the thread structure itself
 */
struct qthread {
};

/* Mutex
 */
struct qthread_mutex {
};

/* Condition variable
 */
struct qthread_cond {
    /* your code here */
};

#endif
