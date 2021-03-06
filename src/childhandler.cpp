//
// Created by connor on 10/26/2020.
//

#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <sstream>
#include <vector>
#include <set>
#include <string>
#include <cstring>
#include "childhandler.h"
#include "errors.h"

std::set<pid_t> PIDS;

char** makeargv(std::string line, int& size)
{
    std::istringstream iss(line);
    std::vector<std::string> argvector;

    while(iss)
    {
        std::string sub;
        iss >> sub;
        argvector.push_back(sub);
    }

    size = argvector.size();
    char** out = new char*[size];

    for(int i = 0; i < size-1; i++)
    {
        out[i] = new char[argvector[i].size()];
        strcpy(out[i], argvector[i].c_str());
    }

    out[size-1] = nullptr;

    return out;
}

void freeargv(char** argv, int size)
{
    for(int i = 0; i < size; i++)
    {
        delete[] argv[i];
    }

    delete[] argv;
}

int forkexec(std::string cmd, int& procCounter)
{
    return forkexec(cmd.c_str(), procCounter);
}

int forkexec(const char* cmd, int& procCounter)
{
    int childargc;

    char** childargv = makeargv(cmd, childargc);

    const pid_t cpid = fork();

    switch(cpid)
    {
        case -1:
            perrorquit();
            return -1;

        case 0:
            if(execvp(childargv[0], childargv) == -1)
            {
                perrorquit();
            }
            return 0;

        default:
            PIDS.insert(cpid);
            procCounter++;
            freeargv(childargv, childargc);
            return cpid;
    }

}

int updatechildcount(int& procCount)
{
    int wstatus;
    pid_t pid;

    switch((pid = waitpid(-1, &wstatus, WNOHANG)))
    {
        case -1:
            perrorquit();
            return -1;

        case 0:
            return 0;

        default:
            PIDS.erase(pid);
            procCount--;
            return pid;
    }
}

int waitforanychild(int& procCount)
{
    int wstatus;
    pid_t pid;

    switch((pid = waitpid(-1, &wstatus, 0)))
    {
        case -1:
            perrorquit();
            return -1;

        default:
            PIDS.erase(pid);
            procCount--;
            return pid;
    }
}

void killall()
{
    for (int p : PIDS)
    {
        if(kill(p, SIGINT) == -1)
        {
            perrorquit();
        }
    }
}
