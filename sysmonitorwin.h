#ifndef SYSMONITORWIN_H
#define SYSMONITORWIN_H

#include "sysmonitor.h"
#include "perfhelper.h"

/*! Platform dependent implementation of abstract class SysMonitor.*/
class SysMonitorWin : public SysMonitor
{
public:
    SysMonitorWin();
    ~SysMonitorWin();

    // SysMonitor interface

public:
    void initialize(double thresholdsConfig[], std::string &filePathFromConfig);
    void getCPULoad(double *value);
    void getPerCPULoad(std::vector<double> *value);
    void getNetworkLoad(double *absValue, double *relValue, double *bandWidth);
    void getMemoryFree(double *absValue, double *relValue, double *total);
    void getMemoryUsed(double *absValue, double *relValue, double *total);
    void SetAlertCores(std::vector<int> vect);
    void SetIndexOfAdapters(std::vector<int> &adp);
    void ClearInstances();


private:
    PerCPUPerfHelper* _pcpu;
    NetworkPerfHelper* _ph;
    CPUPerfHelper* _cpu;
    MemoryPerfHelper *_mh;
};

#endif // SYSMONITORWIN_H
