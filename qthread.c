/*
 * file:        qthread.c
 * description: simple emulation of POSIX threads
 * class:       CS 7600, Spring 2013
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include "qthread.h"

#define THREADSTACKSIZE 4096

/*
 * do_switch is defined in do-switch.s, as the stack frame layout
 * changes with optimization level, making it difficult to do with
 * inline assembler.
 */
extern void do_switch(void **location_for_old_sp, void *new_value);


/*
 * setup_stack(stack, function, arg1, arg2) - sets up a stack so that
 * switching to it from 'do_switch' will call 'function' with arguments
 * 'arg1' and 'arg2'. Returns the resulting stack pointer.
 *
 * works fine with functions that take one argument ('arg1') or no
 * arguments, as well.
 */
void *setup_stack(int *stack, void *func, void *arg1, void *arg2)
{

   // printf("%d", *stack);
  
    printf("Boom!!!\n");

    int old_bp = (int)stack;	/* top frame - SP = BP */

    *(--stack) = 0x3A3A3A3A;    /* guard zone */
    *(--stack) = 0x3A3A3A3A;
    *(--stack) = 0x3A3A3A3A;

    printf("Bang!!!\n");

    /* this is the stack frame "calling" 'func'
     */
    *(--stack) = (int)arg2;     /* argument */
    *(--stack) = (int)arg1;     /* argument (reverse order) */
    *(--stack) = 0;             /* fake return address (to 'func') */

    /* this is the stack frame calling 'do_switch'
     */
    *(--stack) = (int)func;     /* return address */
    *(--stack) = old_bp;        /* %ebp */
    *(--stack) = 0;             /* %ebx */
    *(--stack) = 0;             /* %esi */
    *(--stack) = 0;             /* %edi */
    *(--stack) = 0xa5a5a5a5;    /* valid stack flag */

    return stack;
}

/* You'll need to do sub-second arithmetic on time. This is an easy
 * way to do it - it returns the current time as a floating point
 * number. The result is as accurate as the clock usleep() uses, so
 * it's fine for us.
 */
static double gettime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec/1.0e6;
}

/* We don't have to define the qthread structure in qthread.h, since
 * the user program only sees pointers to it.
 */
struct qthread {
    /*
     * note - you can't use 'qthread_t*' in this definition, due to C
     * limitations.
     * use 'struct qthread *' instead, which means the same thing:
     *    struct qthread *next;
     */

        long tid;
        struct qthread *prev;
        struct qthread *next;
        void* basePtr;
        void* offsetPtr;
        qthread_attr_t detached;
        short status;
	time_t lastPickupTime;
}*activeThreadList = NULL,  *tail = NULL;

/* A good organization is to keep a pointer to the 'current'
 * (i.e. running) thread, and a list of 'active' threads (not
 * including 'current') which are also ready to run.
 */


/* Note that on startup there's already a thread running thread - we
 * need a placeholder 'struct thread' so that we can switch away from
 * that to another thread and then switch back. 
 */
struct qthread os_thread = {};
qthread_t current = &os_thread;


void findNextRunnableThread(qthread_t nextRunnable) {

        if(activeThreadList != NULL) {

                qthread_t iterator = activeThreadList;

                nextRunnable = NULL;

                while(iterator != NULL) {

                        if(((iterator->status == 1) || (iterator->status == 3))) {
                                if(nextRunnable != NULL) {

                                        if(nextRunnable->lastPickupTime > iterator->lastPickupTime)

                                                nextRunnable = iterator;

                                } else
                                        nextRunnable = iterator;
                        }
                }
        }
}



/* Beware - you cannot use do_switch to switch from a thread to
 * itself. If there are no other active threads (or after a timeout
 * the first scheduled thread is the current one) you should return
 * without switching. (why? because you haven't saved the current
 * stack pointer)
 */

/* qthread_yield - yield to the next runnable thread.
 */
int qthread_yield(void)
{
    
    qthread_t nextRunnableThread = NULL, prev = NULL;
    findNextRunnableThread(nextRunnableThread);
    prev = current;
    current = nextRunnableThread;
    do_switch(prev->basePtr, nextRunnableThread->basePtr);

    return 0;
}

/* Initialize a thread attribute structure. We're using an 'int' for
 * this, so just set it to zero.
 */
int qthread_attr_init(qthread_attr_t *attr)
{
    *attr = 0;
    return 0;
}

/* The only attribute supported is 'detached' - if it is true a thread
 * will clean up when it exits; otherwise the thread structure hangs
 * around until another thread calls qthread_join() on it.
 */
int qthread_attr_setdetachstate(qthread_attr_t *attr, int detachstate)
{
    /* your code here */
    return 0;
}

void insertTCB(qthread_t newThread){

        if(activeThreadList == NULL) {

                activeThreadList = tail = newThread;
                newThread->tid = os_thread.tid + 1;


        } else {

		qthread_t front = activeThreadList;

                newThread->next = front;
                front->prev = newThread;
                front = front->prev;
                activeThreadList = front;
                newThread->tid  = newThread->next->tid + 1;
        }
}

void freeTCB(long toBeDeleted) {

        qthread_t iterator = activeThreadList;

        if(activeThreadList != NULL) {

                while(iterator != NULL) {
                        if(iterator->tid == toBeDeleted) {

                                free(iterator->basePtr);

                                if(iterator->detached) {

                                        iterator->prev->next = iterator->next;
                                        iterator->next->prev = iterator->prev;
                                        free(iterator);

                                } else {

                                        iterator->offsetPtr = NULL;
                                        iterator->status = 4;
                                        iterator->detached = -1;
                                }

                                break;

                        } else
                                iterator = iterator->next;
                }

        } else
                fprintf(stderr, "List Empty!!! Cannot Free the given thread!!!");

}

int isActiveThreadListEmpty() {

        return (activeThreadList == NULL)?1:0;

}

void printActiveThreadList() {

        if(activeThreadList){

                qthread_t temp = activeThreadList;

                while(temp != NULL) {
                        printf("%ld\n", temp->tid);
                        temp = temp->next;
                }
        }
}

void allocateThreadStack(void** basePtr, void** offsetPtr) {

        *basePtr = malloc(4096);
        *offsetPtr = *basePtr + 4096;
}

void createAndSetupTCB(qthread_t currentTCB) {
    printf("in setup and create stack\n");

	currentTCB = (qthread_t)malloc(sizeof(struct qthread)); 
	insertTCB(currentTCB);
    
	allocateThreadStack(&currentTCB->basePtr, &currentTCB->offsetPtr);	

    printf("%p\n", &currentTCB->basePtr);
    printf("%p\n", &currentTCB->offsetPtr);
     
    currentTCB->detached = 0;
    currentTCB->status = 1;
    currentTCB->prev = NULL;
    currentTCB->next = NULL;
}

void initThreadLib() {

	os_thread.tid = 0;
	os_thread.detached = 0;
	os_thread.status = 1;
	os_thread.prev = NULL;
	os_thread.next = NULL;
	allocateThreadStack(&os_thread.basePtr, &os_thread.offsetPtr);

	setup_stack(os_thread.offsetPtr, NULL, NULL, NULL);


}


/* a thread can exit by either returning from its main function or
 * calling qthread_exit(), so you should probably use a dummy start
 * function that calls the real start function and then calls
 * qthread_exit after it returns.
 */

/* qthread_create - create a new thread and add it to the active list
 */
int qthread_create(qthread_t *thread, qthread_attr_t *attr,
                   qthread_func_ptr_t start, void *arg)
{

    printf("test123\n");

    if(isActiveThreadListEmpty())
	initThreadLib();
    printf("done initialize\n");
    printf("%p\n", os_thread.basePtr);
    printf("%p\n", os_thread.offsetPtr);

    qthread_t newTCB = *thread;

    createAndSetupTCB(newTCB);
    setup_stack(newTCB->offsetPtr, NULL,NULL, NULL);

    qthread_yield();

    return 0;
}

/* qthread_exit - sort of like qthread_yield, except we never
 * return. If the thread is joinable you need to save 'val' for a
 * future call to qthread_join; otherwise you can free allocated
 * memory. 
 */
void qthread_exit(void *val)
{
    /* your code here */
}

/* qthread_mutex_init/destroy - initialize (destroy) a mutex. Ignore
 * 'attr' - mutexes are non-recursive, non-debugging, and
 * non-any-other-POSIX-feature. 
 */
int qthread_mutex_init(qthread_mutex_t *mutex, qthread_mutexattr_t *attr)
{
    /* your code here */
    return 0;
}
int qthread_mutex_destroy(qthread_mutex_t *mutex)
{
    /* your code here */
    return 0;
}

/* qthread_mutex_lock/unlock
 */
int qthread_mutex_lock(qthread_mutex_t *mutex)
{
    /* your code here */
    return 0;
}
int qthread_mutex_unlock(qthread_mutex_t *mutex)
{
    /* your code here */
    return 0;
}

/* qthread_cond_init/destroy - initialize a condition variable. Again
 * we ignore 'attr'.
 */
int qthread_cond_init(qthread_cond_t *cond, qthread_condattr_t *attr)
{
    /* your code here */
    return 0;
}
int qthread_cond_destroy(qthread_cond_t *cond)
{
    /* your code here */
    return 0;
}

/* qthread_cond_wait - unlock the mutex and wait on 'cond' until
 * signalled; lock the mutex again before returning.
 */
int qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex)
{
    /* your code here */
    return 0;
}

/* qthread_cond_signal/broadcast - wake one/all threads waiting on a
 * condition variable. Not an error if no threads are waiting.
 */
int qthread_cond_signal(qthread_cond_t *cond)
{
    /* your code here */
    return 0;
}

int qthread_cond_broadcast(qthread_cond_t *cond)
{
    /* your code here */
    return 0;
}

/* POSIX replacement API. These are all the functions (well, the ones
 * used by the sample application) that might block.
 *
 * If there are no runnable threads, your scheduler needs to block
 * waiting for one of these blocking functions to return. You should
 * probably do this using the select() system call, indicating all the
 * file descriptors that threads are blocked on, and with a timeout
 * for the earliest thread waiting in qthread_usleep()
 */

/* qthread_usleep - yield to next runnable thread, making arrangements
 * to be put back on the active list after 'usecs' timeout. 
 */
int qthread_usleep(long int usecs)
{
    /* your code here */
    return 0;
}

/* make sure that the file descriptor is in non-blocking mode, try to
 * read from it, if you get -1 / EAGAIN then add it to the list of
 * file descriptors to go in the big scheduling 'select()' and switch
 * to another thread.
 */
ssize_t qthread_read(int sockfd, void *buf, size_t len)
{
    /* set non-blocking mode every time. If we added some more
     * wrappers we could set non-blocking mode from the beginning, but
     * this is a lot simpler (if less efficient)
     */
    int tmp = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, tmp | O_NONBLOCK);

    return 0;
}

/* like read - make sure the descriptor is in non-blocking mode, check
 * if if there's anything there - if so, return it, otherwise save fd
 * and switch to another thread. Note that accept() counts as a 'read'
 * for the select call.
 */
int qthread_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    return 0;
}

/* Like read, again. Note that this is an output, rather than an input
 * - it can block if the network is slow, although it's not likely to
 * in most of our testing.
 */
ssize_t qthread_write(int sockfd, const void *buf, size_t len)
{
    return 0;
}


void *test_func(void *arg) { 

    printf("in thread");
    sleep(60);
    printf("exiting thread");
    return arg;
}

int main() {

	void *p = malloc(4096);


	qthread_t *t1;
	qthread_create(&t1, NULL, test_func, NULL);
printf("starting");

}

