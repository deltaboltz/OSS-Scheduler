//
// Created by Connor on 11/4/2020.
//
#include "clock_work.h"
#include <ctime>


float clck::tofloat()
{
    return (float)this->clockSec + (float)this->clockNano/(float)1e9;
}
std::string clck::toString()
{
    int padding = 9 - std::to_string(this->clockNano).size();
    std::string rep = std::to_string(this->clockSec) + ".";

    while(padding--)
    {
        rep += "0";
    }
    return rep + std::to_string(this->clockNano);
}

void clck::increment(long nanoSeconds)
{
    this->clockNano += nanoSeconds;
    while(this->clockNano > 1e9)
    {
        this->clockNano -= 1e9;
        this->clockSec += 1;
    }
}

void clck::decrement(long nanoSeconds)
{
    this->clockNano -= nanoSeconds;
    while(this->clockNano < 0)
    {
        this->clockNano += 1e9;
        this->clockSec -= 1;
    }
    if(this->clockSec)
    {
        this->clockSec = 0;
        this->clockNano = 0;
    }
}

float clck::nextrand(long maxNano) {
    clck val;
    val.clockSec = this->clockSec;
    val.clockNano = this->clockNano;
    val.increment(rand() & maxNano);
    return val.tofloat();
}

void clck::set(float time)
{
    this->clockSec = (long)time;
    this->clockNano = (long)((time - this->clockSec) * 1e9);
}
void clck::set(std::string time)
{
    float time1 = std::stof(time);
    this->set(time1);
}

std::string epochlogid()
{
    return std::to_string(time(0));
}


