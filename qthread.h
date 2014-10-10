/*
 * file:        qthread.h
 * description: interface definitions for CS7600 Pthreads assignment
 *
 * Peter Desnoyers, Northeastern CCIS, 2013
 * $Id: $
 */
#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

/* function pointer w/ signature 'void *f(void*)'
 */
typedef void *(*qthread_func_ptr_t)(void*);  

/* forward definitions. Note that the full structure definition isn't
 * needed outside of qthread.c, as no other code looks inside the
 * structures. (so define the contents of these structures at the top
 * of qthread.c)  
 */
struct qthread;
struct qthread_mutex;
struct qthread_cond;

struct qthreadList;

/* You are free to change these type definitions, but the ones
 * provided can work fairly well.
 */
typedef int qthread_attr_t;       /* need to support detachstate */
typedef struct qthread *qthread_t;

typedef void qthread_mutexattr_t; /* no mutex attributes needed */
typedef struct qthread_mutex qthread_mutex_t;

typedef void qthread_condattr_t;  /* no cond attributes needed */
typedef struct qthread_cond qthread_cond_t;

int  qthread_yield(void);

int  qthread_attr_init(qthread_attr_t *attr);
int  qthread_attr_setdetachstate(qthread_attr_t *attr, int detachstate);
int  qthread_create(qthread_t *thread, qthread_attr_t *attr,
                    qthread_func_ptr_t start, void *arg);
int  qthread_join(qthread_t thread, void **retval);
void qthread_exit(void *val);
int  qthread_cancel(qthread_t thread);

#define QTHREAD_CANCELLED ((void*)2)

int qthread_mutex_init(qthread_mutex_t *mutex, qthread_mutexattr_t *attr);
int qthread_mutex_destroy(qthread_mutex_t *mutex);
int qthread_mutex_lock(qthread_mutex_t *mutex);
int qthread_mutex_unlock(qthread_mutex_t *mutex);

int qthread_cond_init(qthread_cond_t *cond, qthread_condattr_t *attr);
int qthread_cond_destroy(qthread_cond_t *cond);
int qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex);
int qthread_cond_signal(qthread_cond_t *cond);
int qthread_cond_broadcast(qthread_cond_t *cond);

/* POSIX replacement API. Not general, but enough to run a
 * multi-threaded webserver. 
 */
int     qthread_usleep(long int usecs);
ssize_t qthread_read(int fd, void *buf, size_t len);
struct sockaddr;
int     qthread_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t qthread_write(int fd, const void *buf, size_t len);

struct qthread {
    /*
     * note - you can't use 'qthread_t*' in this definition, due to C
     * limitations.
     * use 'struct qthread *' instead, which means the same thing:
     *    struct qthread *next;
     */

        long tid;
        qthread_t prev;
        qthread_t next;
        void* basePtr;
        void* offsetPtr;
        qthread_attr_t detached;
        short status;
        double wakeupTime;
        void* exitStatus;
	short condVarWaitStatus;
};


struct qthread_mutex {
        short state;
};


/* Condition variable
 */
struct qthread_cond {

    struct qthreadList *waitingList;

};


struct qthreadList {
	qthread_t thread;
        struct qthreadList *next;
};

#endif /* __QTHREAD_H__ */
