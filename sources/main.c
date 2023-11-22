#include "stdio.h"
#include <limits.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdbool.h>

//#include <linux/rpmsg.h>

//write a fixed message in ttyRPMSG0

#include <stdio.h>

#include <unistd.h>

#include <fcntl.h>
#include <time.h>

#define TTY_RPMSG0 "/dev/ttyRPMSG0"
#define TTY_RPMSG1 "/dev/ttyRPMSG1"

//#define DEBUG_MESSAGES

struct period_info {
        struct timespec next_period;
        long period_ns;
};
 
static void inc_period(struct period_info *pinfo) 
{
        pinfo->next_period.tv_nsec += pinfo->period_ns;
 
        while (pinfo->next_period.tv_nsec >= 1000000000) {
                /* timespec nsec overflow */
                pinfo->next_period.tv_sec++;
                pinfo->next_period.tv_nsec -= 1000000000;
        }
}


void *thread_func(void *data)
{    
    //setup timing structures
    struct timespec time_keeper;
    struct timespec time_write;
    struct timespec time_read;
    struct period_info wait_time;

    //specify wait period for task
    wait_time.period_ns = 100000;

    //variables for the control of read-write operation
    bool waiting = false;
    int err = -1;

    //dummy message to send and buffer to read from file
    char msg [] = "echo";
    char buffer [50];

    //open RPMsg file
    int fd = open(TTY_RPMSG0, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);

    //log time of start and setup first trigger point
    clock_gettime(CLOCK_MONOTONIC, &time_keeper);
    clock_gettime(CLOCK_MONOTONIC, &wait_time.next_period);
    printf("Time of start [us]: %ld \n",time_keeper.tv_nsec/1000);

    //simple print for debug
    //int err =    write(fd, msg, sizeof(msg));
    //printf("err: %d", err);
    
    while(1)
    {
        //Debug print for iteration cycle time
        long int prev_time = time_keeper.tv_nsec;
        clock_gettime(CLOCK_MONOTONIC, &time_keeper);
        clock_gettime(CLOCK_MONOTONIC, &wait_time.next_period);
        printf("Iteration time: %ld\n", (time_keeper.tv_nsec - prev_time)/1000);

        //write to RPMsg if not already waiting for message
        if(!waiting)
            err = write(fd, msg, sizeof(msg));
        else
            err = -1;

        //save time of write operation
        if (err != -1)
        {
            waiting = true;
            clock_gettime(CLOCK_MONOTONIC, &time_write);
        }

        //read from RPMsg if waiting for response
        if(waiting)
            err = read(fd,&buffer,sizeof(buffer));
        else
            err = -1;

        //save time of read operation
        if (err != -1)
        {
            clock_gettime(CLOCK_MONOTONIC, &time_read);

            //log calculated RTT
            printf("RTT [us]: %ld \n", (time_read.tv_nsec - time_write.tv_nsec)/1000);
            waiting = false;
        }

        //calculate next thread wakeup time and put thread to sleep
        inc_period(&wait_time); 
        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wait_time.next_period, NULL);
    }
 
    //close the device
    close(fd);
    return NULL;
}

int main(int argc, char * argv [])
{

    struct sched_param param;
    pthread_attr_t attr;
    pthread_t thread;
    int ret;

    /* Lock memory */
    if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            printf("mlockall failed: %m\n");
            exit(-2);
    }

    /* Initialize pthread attributes (default values) */
    ret = pthread_attr_init(&attr);
    if (ret) {
            printf("init pthread attributes failed\n");
            goto out;
    }

    /* Set a specific stack size  */
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + 1000);
    if (ret) {
        printf("pthread setstacksize failed\n");
        goto out;
    }

    /* Set scheduler policy and priority of pthread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    if (ret) {
            printf("pthread setschedpolicy failed\n");
            goto out;
    }
    param.sched_priority = 99;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
            printf("pthread setschedparam failed\n");
            goto out;
    }
    /* Use scheduling parameters of attr */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
            printf("pthread setinheritsched failed\n");
            goto out;
    }

    /* Create a pthread with specified attributes */
    ret = pthread_create(&thread, &attr, thread_func, NULL);
    if (ret) {
            printf("create pthread failed\n");
            goto out;
    }

    /* Join the thread and wait until it is done */
    ret = pthread_join(thread, NULL);
    if (ret)
            printf("join pthread failed: %m\n");
 
out:
    //printf("Done all I could"); //Debug print
    return ret;

}
