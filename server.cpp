#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include <unistd.h>
#include <iostream>
#include "sys/types.h"
#include "sys/sysinfo.h"
using namespace std;

class ram
{
private:
    struct sysinfo memInfo;
    long long totalVirtualMem, totalPhysMem, physMemUsed;

public:
    ram(){
        run();
    }

    void run()
    {
        sysinfo(&memInfo);
        totalVirtualMem = memInfo.totalram;
        totalVirtualMem += memInfo.totalswap;
        totalVirtualMem *= memInfo.mem_unit;

        totalPhysMem = memInfo.totalram;
        totalPhysMem *= memInfo.mem_unit;

        physMemUsed = memInfo.totalram - memInfo.freeram;
        physMemUsed *= memInfo.mem_unit;
    }
    
    double get_mem(){
        run();
        return (1-(double)physMemUsed/totalPhysMem)*100;
    }
};

static unsigned long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
void init()
{
    FILE *file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &lastTotalUser, &lastTotalUserLow,
           &lastTotalSys, &lastTotalIdle);
    fclose(file);
}

double getCurrentValue()
{
    double percent;
    FILE *file;
    unsigned long long totalUser, totalUserLow, totalSys, totalIdle, total;

    file = fopen("/proc/stat", "r");
    fscanf(file, "cpu %llu %llu %llu %llu", &totalUser, &totalUserLow,
           &totalSys, &totalIdle);
    fclose(file);

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle)
    {
        // Overflow detection. Just skip this value.
        percent = -1.0;
    }
    else
    {
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}

int main()
{
    ram k;
    while (1)
    {
        usleep(100 * 1000);
        cout << getCurrentValue() << endl;
        cout << k.get_mem() << endl << endl;
    }
}