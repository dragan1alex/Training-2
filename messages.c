//build with -lrt
#include<stdlib.h>
#include<stdio.h>
#include<pthread.h>
#include<fcntl.h>
#include<sys/types.h>
#include<mqueue.h>
#include<unistd.h>
#include<inttypes.h>
#include<string.h>

#define BUF_SIZE 9000

struct mq_attr at;


int main(int argc, char const *argv[])
{
    mqd_t m, rm; //m is the server handle, rm is the client handle
    printf("App started.\nTo terminate, press CTRL+C");
    char q[BUF_SIZE];
    char address[30];
    uint8_t command;
    at.mq_maxmsg = 30;
    at.mq_msgsize = BUF_SIZE;
    at.mq_flags = 0;
    //check in what mode the program was started (with arguments = server, no arguments = client)
    if(argc > 1)
    {
        printf("\nRunning in server mode.");
        m = mq_open("/tserver", O_CREAT | O_RDWR, 0666, &at);
    }
    else
    {
        printf("\nRunning in client mode.");
        m = mq_open("/tserver", O_RDWR);
    }
    
    if(m == (mqd_t)-1)
    {
        printf("\nCrytical error: cannot open the channel to the server.\nExiting...\n");
        return 1;
    }

    struct mq_attr attr; //used for checking the status of the message queues

    if(argc > 1) //server mode
    {
        printf("\nWill serve ticket numbers in ascending order to the clients.");
        uint32_t i = 1; //a counter for sending a unique ticket number to the clients
        while(1)
        {
            mq_getattr(m, &attr);
            printf("\n------------------------------\n");
            printf("\nWaiting for a request...");
            while(attr.mq_curmsgs < 1)
            {
                mq_getattr(m, &attr);
            }
            //when there is a message in the queue, capture and decode it
            mq_receive(m, q, BUF_SIZE, 0);
            //we don't need to check the validity of the message
            //if it is invalid the server won't be able to open the message queue and will give up
            strcpy(address, strtok(q, " "));
            command = atoi(strtok(NULL, " "));
            printf("\nReceived command %d from %s", command, address);
            if(command == 1)
            {
                printf("\nTrying to open a message queue to the client...");
                rm = mq_open(address, O_RDWR);
                if(rm != (mqd_t)-1)
                {
                    printf("done.");
                    printf("\nSending %d", i);
                    sprintf(q, "%d", i);
                    mq_send(rm, q, BUF_SIZE, 0);
                    i++;
                    mq_close(rm);
                }
                else
                {
                    printf("error!\nThe packet was corrupted or the message queue is unavailable.");
                }
            }
        }
    }
    else //client mode
    {
        //the address is generated from the thread ID of the running client, in the following format: "/<thread id>"
        printf("\nGenerating a unique address: ");
        sprintf(address, "/%lu", pthread_self());
        printf("%s", address);
        printf("\nWill ask for a ticket number every 1-5 seconds");
        while(1)
        {
            //pseudo-random sleep timer for 1-5 seconds
            sleep(1 + rand() % 5);

            //craft the packet with the return address and command, separated by a space
            command = 1;
            sprintf(q, "%s %d", address, command);

            //open a message queue for receiving the response from the server
            printf("\n------------------------------\n");
            printf("\nOpening a message queue");
            rm = mq_open(address, O_CREAT | O_RDWR, 0666, &at);
            if(rm == (mqd_t)-1)
            {
                printf("\nCouldn't create the message queue, maybe check for permissions?");
                printf("\nExiting, goodbye!\n");
                return 0;
            }

            //send the request to the server
            printf("\nSending request %s", q);
            mq_send(m, q, BUF_SIZE, 0);
            q[0] = '\0';

            //wait for a response
            mq_getattr(rm, &attr);
            while(attr.mq_curmsgs < 1)
            {
                mq_getattr(rm, &attr);
            }

            //receive the response and display it
            mq_receive(rm, q, BUF_SIZE, 0);
            printf("\nReceived %s", q);

            mq_close(rm); //close the queue
            mq_unlink(address); //delete it
            printf("\nSleeping...");
        }
    }
    return 0;
}
