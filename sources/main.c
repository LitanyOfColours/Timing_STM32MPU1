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


void *thread_func(void *data)
{    
    struct timespec time_keeper;
    struct timespec time_write;
    struct timespec time_read;
    struct timespec wait_time;

    wait_time.tv_sec = 0;
    wait_time.tv_nsec = 1000000000;

    bool waiting = false;
    int err = -1;

    char msg [] = "echo";
    char buffer [50];

    int fd = open(TTY_RPMSG0, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);

    clock_gettime(CLOCK_MONOTONIC, &time_keeper);
    printf("Time of start [us]: %ld \n",time_keeper.tv_nsec/1000);

    //simple print for debug
    //int err =    write(fd, msg, sizeof(msg));
    //printf("err: %d", err);
    
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC, &time_keeper);
        printf("I am still alive at time: %ld\n", time_keeper.tv_nsec/1000);

        if(!waiting)
            err = write(fd, msg, sizeof(msg));
        else
            err = -1;

        if (err != -1)
        {
            waiting = true;
            clock_gettime(CLOCK_MONOTONIC, &time_write);
        }

        if(waiting)
            err = read(fd,&buffer,sizeof(buffer));
        else
            err = -1;

        if (err != -1)
        {
            clock_gettime(CLOCK_MONOTONIC, &time_read);

            printf("RTT [us]: %ld \n", (time_read.tv_nsec - time_write.tv_nsec)/1000);
            waiting = false;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &wait_time, NULL);
    }
    /*
    while(1)
    {

        int err = read(fd,&buffer,sizeof(buffer));
        
        if (err != -1)
        {
            clock_gettime(CLOCK_MONOTONIC, &time_read);

            printf("RTT [us]: %ld \n", (time_read.tv_nsec - time_write.tv_nsec)/1000);

            break;
        }

    }*/
        /* Do RT specific stuff here */
    /*


    while(1)
    {
        
        //write a simple message and its length
        //int err =    write(fd, msg, sizeof(msg));

        while(1)
        {

            int err = write(fd, msg, sizeof(msg));

            #ifdef DEBUG_MESSAGES
            printf("Attempting write, err: %d\n",err);
            #endif
            
            if (err != -1)
            {
                clock_gettime(CLOCK_MONOTONIC, &time_write);
                #ifdef DEBUG_MESSAGES
                printf(buffer);
                printf("\n");
                #endif
                
                break;
            }

        }

        while(1)
        {

            int err = read(fd,&buffer,sizeof(buffer));

            #ifdef DEBUG_MESSAGES
            printf("Attempting read, err: %d \n", err);
            #endif
            
            if (err != -1)
            {
                clock_gettime(CLOCK_MONOTONIC, &time_read);
                #ifdef DEBUG_MESSAGES
                printf(buffer);
                #endif

                printf("RTT [us]: %ld \n", (time_read.tv_nsec - time_write.tv_nsec)/1000);

                break;
            }

        }
    }
    */

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
    printf("Done all I could");
    return ret;
/*
    struct timespec time_keeper;
    struct timespec time_write;
    struct timespec time_read;

    clock_gettime(CLOCK_REALTIME, &time_keeper);
    printf("Time of start [us]: %ld \n",time_keeper.tv_nsec/1000);
    char msg [] = "echo";
    char buffer [50];


    //open the device
    int fd = open(TTY_RPMSG0, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);


    while(1)
    {
        
        //write a simple message and its length
        //int err =    write(fd, msg, sizeof(msg));

        #ifdef DEBUG_MESSAGES
        printf("Attempting write, err: %d\n",err );
        #endif

        while(1)
        {

            int err = write(fd, msg, sizeof(msg));

            #ifdef DEBUG_MESSAGES
            printf("Attempting write, err: %d\n",err);
            #endif
            
            if (err != -1)
            {
                clock_gettime(CLOCK_REALTIME, &time_write);
                #ifdef DEBUG_MESSAGES
                printf(buffer);
                printf("\n");
                #endif
                
                break;
            }

        }

        while(1)
        {

            int err = read(fd,&buffer,sizeof(buffer));

            #ifdef DEBUG_MESSAGES
            printf("Attempting read, err: %d \n", err);
            #endif
            
            if (err != -1)
            {
                clock_gettime(CLOCK_REALTIME, &time_read);
                #ifdef DEBUG_MESSAGES
                printf(buffer);
                #endif

                printf("RTT [us]: %ld \n", (time_read.tv_nsec - time_write.tv_nsec)/1000);

                break;
            }

        }
    }


    //close the device
    close(fd);

    return 0;*/

}
