
#include "sysmonitorwin.h"
#include "config.h"
#include "perfhelper.h"
#include<iostream>


using namespace std;

SysMonitorWin::SysMonitorWin()
{
    _ph = nullptr;

}

SysMonitorWin::~SysMonitorWin()
{
    delete _ph;
    delete _cpu;
    delete _mh;
    delete _pcpu;
}

void SysMonitorWin::initialize(double thresholdsConfig[],std::string &filePathFromConfig)
{
  LPCSTR lpszTest =LPCSTR(filePathFromConfig.c_str());
  setDiskDirectory(lpszTest);


  // assignment of thresholds
  _theCPU = thresholdsConfig[0];
  _thwCPU = thresholdsConfig[5];
  _thePerCPU = thresholdsConfig[1];
  _thwPerCPU = thresholdsConfig[6];
  _theFS = thresholdsConfig[2];
  _thwFS =  thresholdsConfig[7];
  _theMem =  thresholdsConfig[3];
  _thwMem =  thresholdsConfig[8];
  _theNetwork =  thresholdsConfig[4];
  _thwNetwork = thresholdsConfig[9];


  _maxCPU = 1.0;
  _maxPerCPU=1.0;
  getMemoryFree(nullptr,nullptr, &_maxMem);
  getFileSystemFree(_path, nullptr, nullptr, &_maxFS);

  _ph = new NetworkPerfHelper();
  _cpu = new CPUPerfHelper();
  _pcpu=new PerCPUPerfHelper();

  SYSTEM_INFO systemInfo;
  GetSystemInfo( &systemInfo );

  int n = systemInfo.dwNumberOfProcessors;

  v.clear();
  for(n;n>0;n--)
  {
    v.push_back(0);
  }

  _mh = new MemoryPerfHelper();

  _cpu->GetCPULoad();
  _pcpu->GetPerCpuLoad();
  _maxMem = _mh->GetMemoryTotal();

  getNetworkLoad(nullptr, nullptr, &_maxNetwork);
}

void SysMonitorWin::getCPULoad(double *value)
{
    if(value)
        *value = _cpu->GetCPULoad();
}
void SysMonitorWin::getPerCPULoad(std::vector<double> *value)
{
    if(value)
        *value = _pcpu->GetPerCpuLoad();
}

void SysMonitorWin::getNetworkLoad(double *absValue, double *relValue, double *bandWidth)
{
  if(absValue || relValue || bandWidth)
  {
    double speed, bw;
    _ph->GetNetworkLoad(&speed, &bw);

    if(absValue)
    {
      *absValue = speed;
    }
    if(bandWidth)
    {
      *bandWidth =  bw;

    }
    if(relValue)
    {
      if(bw)
      {
        *relValue = (8*speed*100)/bw;
      }
      else
        *relValue = 0;

    }
  }
}


void SysMonitorWin::getMemoryFree(double *absValue, double *relValue, double *total)
{
    double m = _mh->GetMemoryLoad();
    if(absValue)
    {
        *absValue = _maxMem - m;
    }
    if(relValue)
    {
        *relValue = (_maxMem - m)/_maxMem;
    }
    if(total)
    {
        *total = _maxMem;
    }
}

void SysMonitorWin::getMemoryUsed(double *absValue, double *relValue, double *total)
{
    double m = _mh->GetMemoryLoad();
    if(absValue)
    {
        *absValue =  m;
    }
    if(relValue)
    {
        *relValue =  m/_maxMem;
    }
    if(total)
    {
        *total = _maxMem;
    }
}


void SysMonitorWin::SetAlertCores(std::vector<int> vect)
{
  _pcpu->setV(vect);
}

void SysMonitorWin::SetIndexOfAdapters(std::vector<int> &adp)
{
  _ph->setIndexOfAdp(adp);

}

void SysMonitorWin::ClearInstances()
{
  _ph= new NetworkPerfHelper();

}


