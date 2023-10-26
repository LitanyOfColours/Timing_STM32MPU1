#include "stdio.h"
//#include <linux/rpmsg.h>

//write a fixed message in ttyRPMSG0

#include <stdio.h>

#include <unistd.h>

#include <fcntl.h>
#include <time.h>

#define TTY_RPMSG0 "/dev/ttyRPMSG0"
#define TTY_RPMSG1 "/dev/ttyRPMSG1"

//#define DEBUG_MESSAGES

int main()

{


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

        /*#ifdef DEBUG_MESSAGES
        printf("Attempting write, err: %d\n",err );
        #endif*/

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

    return 0;

}
