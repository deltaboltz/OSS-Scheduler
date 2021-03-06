//
// Created by Connor on 10/27/2020.
//

#include <iostream>
#include <string>

#include <unistd.h>
#include <csignal>
#include <vector>

#include "errors.h"
#include "sharedmemory.h"

#include "scheduler.h"
#include "clock_work.h"

volatile bool earlyquit = false;

void sigHandle(int signum)
{
    exit(-1);
}

int main(int argc, char **argv)
{
    signal(SIGINT, sigHandle);

    setupprefix(argv[0]);

    srand(getpid());

    clck* shclock = (clck*)shmlook(0);
    pcb* pcbtable = (pcb*)shmlook(1);

    int pcbnum = std::stoi(argv[1]);
    //std::cout << "PCB #" << pcbnum << " started\n"; //used for debugging

    pcbmsgbuffer* msg = new pcbmsgbuffer;

    //Do a while loop that is essentially infinite
    while(1)
    {
        //The mtype needs to be unique to each child to send a recieve to proper children
        msg->mtype = pcbnum + 3;
        msgreceive(2, msg);

        //If the msgrcvd is the wrong mtype then we error out
        if(msg->data[PCBNUM] != pcbnum)
        {
            std::cerr << "received message not intended for child " << std::to_string(msg->data[PCBNUM]);
            perrorquit();
        }

        //If we block then we have a random time to stop "wait for an event"
        if(pcbtable[pcbnum].blockStart != 0)
        {
            pcbtable[pcbnum].blockTime += shclock->clockSec * 1e9 + shclock->clockNano - pcbtable[pcbnum].blockStart - 0;

            pcbtable[pcbnum].blockStart = 0;
        }

        //Set the mtype to 1 to send to oss (1 is reserved for oss)
        msg->mtype = 1;

        //Check to see if we will jsut terminate right away and how much time we used before we did
        if((rand() % 10) < 1)
        {
            pcbtable[pcbnum].burstTime = rand() % msg->data[TIMESLICE];
            msg->data[STATUS] = TERM;

            //Send the message to oss that we termed
            msgsend(2, msg);

            pcbtable[pcbnum].sysTime = pcbtable[pcbnum].burstTime + shclock->clockSec * 1e9 + shclock->clockNano - pcbtable[pcbnum].inceptTime;

            //detach all the shared memory for this child
            shmdetach(shclock);
            shmdetach(pcbtable);
            exit(0);
        }
        //Non-Terminating
        else
        {
            /*
             * Three options
             *
             * 0) Use all time slice and expire
             *
             * 1) We get blocked and block for a random time, move to blockQ
             *
             * 2) We preempt the process and only use [1,99]% of our time slice allowed and move to next Q
             */
            int decision = rand() % 3;
            if(decision == 0)
            {
                pcbtable[pcbnum].burstTime = msg->data[TIMESLICE];
                msg->data[STATUS] = RUN;
                msgsend(2, msg);
            }
            else if(decision == 1)
            {
                pcbtable[pcbnum].burstTime = rand() % msg->data[TIMESLICE];
                msg->data[STATUS] = BLOCK;
                msgsend(2, msg);
                pcbtable[pcbnum].blockStart = shclock->clockSec * 1e9 + shclock->clockNano;

                float wakeuptime = shclock->nextrand(3e9);

                while(shclock->tofloat() < wakeuptime);

                msg->mtype = 2;
                msg->data[STATUS] = UNBLOCK;
                msgsend(2, msg);

                msg->mtype = pcbnum+3;
            }
            else if(decision == 2)
            {
                pcbtable[pcbnum].burstTime = msg->data[TIMESLICE] / 100 * (1 + rand() % 99);
                msg->data[STATUS] = PREEMTED;
                msgsend(2, msg);
            }
        }
    }
}
