/*
 * file:        test1.c
 * description: test file for qthreads (homework 1)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "qthread.h"
#include <sys/types.h>      
#include <netinet/in.h>     
#include <sys/socket.h>     /* for socket system calls   */
#include <arpa/inet.h>      /* for socket system calls (bind)  */

typedef struct qthread_mutex qthread_mutex_t;

/* 1. create and join. Create N threads, which don't do anything
 * except return a value. (and possibly print something) Call
 * qthread_join() to wait for each of them.
 */
void *f1(void *arg) { return arg; }
void test1(void)
{
    qthread_t t[10];
    int i, j;
    for (i = 0; i < 10; i++)
        qthread_create(&t[i], NULL, f1, (void*)i);
    for (i = 0; i < 10; i++) {
        qthread_join(t[i], (void**)&j);
        assert(i == j);
    }
    printf("test 1 OK\n");
}

/* 2. mutex and sleep.
 * initialize a mutex
 * Create thread 1, which locks a mutex and goes to sleep for a
 * second or two using qthread_usleep.
 *   (you can wait for this by calling qthread_yield repeatedly,
 *    waiting for thread 1 to set a global variable)
 * threads 2..N try to lock the mutex
 * after thread 1 wakes up it releases the mutex; 
 * threads 2..N each release the mutex after getting it.
 * run with N=2,3,4
 */
int t1rdy;
qthread_mutex_t m;
void *f2(void *v)
{
    qthread_mutex_lock(&m);
    t1rdy = 1;
    qthread_usleep(1);
    qthread_mutex_unlock(&m);

    return 0;
}

void *f3(void *v)
{
    qthread_mutex_lock(&m);
    printf("f3\n");
    qthread_mutex_unlock(&m);
    return 0;
}
    
void test2(void)
{
    qthread_t t0, t[10];
    int i;
    
    qthread_mutex_init(&m, NULL);
    qthread_create(&t0, NULL, f2, NULL);
    while (!t1rdy)
        qthread_yield();
    for (i = 0; i < 4; i++)
        qthread_create(&t[i], NULL, f3, NULL);

    void *val;
    qthread_join(t0, &val);
    for (i = 0; i < 4; i++)
        qthread_join(t[i], &val);
    
    printf("test 2 done\n");
}

int N = 10;
int var = 0;
qthread_cond_t t1;


void *f4(void *v) {


	qthread_mutex_lock(&m);
	t1rdy++;

	printf("count: %d\n", t1rdy);
	if(t1rdy == N)
		var = 1;

	while(!var) 
		qthread_cond_wait(&t1, &m);

	qthread_cond_signal(&t1);
	printf("count: %d\n", t1rdy);
	t1rdy--;
	qthread_mutex_unlock(&m);
}

void *f5(void *v) {
	char a[100];
	int read_fd = *(int*)v;
	qthread_read(read_fd,(void*)a,100);

}

void *f6(void *v) {

	qthread_usleep(0);

	int write_fd = *(int*)v;
	//srand (10);
	int rnm = rand()%10 ;
	char buf[100];
	
	//printf("Random: %d: \n", rnm);

	if (rnm != 7){
		sprintf(buf, "%d", -1);
		//printf("String: %s: \n", buf);
		qthread_write(write_fd,(void*)buf,100);
	}
	else{
		sprintf(buf,"%d", rnm);
		//printf("String: %s: \n", buf);
		qthread_write(write_fd,(void*)buf,100);
	}



}

int main(int argc, char **argv)
{


    /* Here are some suggested tests to implement. You can either run
     * them one after another in the program (cleaning up threads
     * after each test) or pass a command-line argument indicating
     * which test to run.
     * This may not be enough tests to fully debug your assignment,
     * but it's a good start.
     */

 //   test1();
 //   test2();
    
    /* 3. condvar and sleep.
     * initialize a condvar and a mutex
     * create N threads, each of which locks the mutex, increments a
     *   global count, and waits on the condvar; after the wait it
     *   decrements the count and releases the mutex.
     * call qthread_yield until count indicates all threads are waiting
     * call qthread_signal, qthread_yield until count indicates a
     *   thread has left. repeat.
     */

/*
     t1rdy = 0;
     qthread_cond_init(&t1, NULL);

    qthread_t t[10];
    int i, j;
    for (i = 0; i < 10; i++){
        qthread_create(&t[i], NULL, f4, NULL);
    }
    for (i = 0; i < 10; i++) {
        qthread_join(t[i], NULL);
    }
     qthread_cond_destroy(&t1);

     printf("Final Count: %d\n", t1rdy);

     printf("Test 3 OK\n");

*/

    /* 4. read/write
     * create a pipe ('int fd[2]; pipe(fd);')
     * create 2 threads:
     * one sleeps and then writes to the pipe
     * one reads from the pipe. [this tests blocking read]
     */


    int fd[2];
    pipe(fd);
         
    qthread_t f[2];

    qthread_create(&f[1], NULL, f6, (void*)&fd[1]);
    qthread_create(&f[0], NULL, f5,(void*)&fd[0]);
    //qthread_create(&f[1], NULL, f6, (void*)&fd[1]);

    qthread_join(f[0], NULL);
    qthread_join(f[1], NULL);

    close(fd[0]);
    close(fd[1]);

/*    
    unsigned int          server_s;               // Server socket descriptor
    struct sockaddr_in    server_addr;            // Server Internet address
    struct sockaddr_in    client_addr;            // Client Internet address
    struct in_addr        client_ip_addr;         // Client IP address
    int                   addr_len;               // Internet address length
    unsigned int    client_s;       
    struct hostent *he;

    client_s = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, '0', sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //inet_pton(AF_INET, INADDR_ANY ,&server_addr.sin_addr);

    addr_len = sizeof(server_addr);
    connect(client_s,(struct sockaddr *)&server_addr, addr_len);

    printf("connection established\n");
    char *msg = "GET /README.md";
    int len, bytes_sent;
    len = strlen(msg);
    printf("length of msg: %d\n", len);
    //bytes_sent = send(sockfd, msg, len, 0);
    //send(sockfd, buf, len, 0);
    bytes_sent = qthread_write(client_s, msg, len);
    printf("bytes sent: %d\n", bytes_sent);

    char buf[100];

    int numbytes;
    if ((numbytes = recv(client_s, buf, 100-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    buf[numbytes] = '\0';

    printf("client: received '%s'\n",buf);
  */  
}
