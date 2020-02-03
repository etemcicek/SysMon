//me
#include "sysmonitor.h"

#include <QApplication>
#include <QTimer>
#include <QtGlobal>
#include<iostream>

//CLS
#include <oraCentralLoggingService.h>

#ifdef  Q_OS_WIN
#include "sysmonitorwin.h"
#endif

 char const  SysMonitor::CPUSection[] = "SysMonitor/CPU/";
 char const  SysMonitor::PerCPUSection[]= "SysMonitor/PerCPU/" ;
 char const  SysMonitor::MemorySection[]= "SysMonitor/Memory/";
 char const  SysMonitor::NetworkSection[] = "SysMonitor/Network/";
 char const  SysMonitor::FileSystemSection[]= "SysMonitor/FileSystem/";
 char const  SysMonitor::Error[] = "error";
 char const  SysMonitor::Warning[]= "warning";
using namespace std;

SysMonitor* SysMonitor::getMonitor()
{
#ifdef  Q_OS_WIN
static SysMonitorWin m;
#endif
return &m;
}

SysMonitor::~SysMonitor()
{

}

void SysMonitor::correct_error_threshold(double &warn, double &err, Counter c)
{
    if(warn > err)
    {
        err = warn;
         QString es = get_section_string(c, Error);
    }
}

void SysMonitor::correct_warning_threshold(double& warn, double& err, SysMonitor::Counter c)
{
    if(warn > err)
    {
         warn = err;
         QString es = get_section_string(c, Warning);
    }
}

void SysMonitor::SetWarningThreshold(double t, SysMonitor::Counter c, bool bAbsolute)
{
   QString s = get_section_string(c, Warning);
   if(t<0)
   {
       t=0;
   }
   if(bAbsolute)
   {
       // error value must be not less than warning value
       switch (c)
       {
       case Counter::CPU:
           if( t > _maxCPU)
           {
               t = _maxCPU;
           }
           _thwCPU = t;
           correct_error_threshold(_thwCPU, _theCPU, c);
           break;
       case Counter::PerCPU:
           if( t > _maxPerCPU)
           {
               t = _maxPerCPU;
           }
           _thwPerCPU = t;
           correct_error_threshold(_thwPerCPU, _thePerCPU, c);
           break;
       case Counter::FileSystemFree:
           if(t > _maxFS) t = _maxFS;
           _thwFS = (_maxFS - t)/_maxFS;
           correct_error_threshold(_thwFS, _theFS, c);
           break;
       case Counter::FileSystemUsed:
           if(t > _maxFS) t = _maxFS;
           _thwFS =  t/_maxFS;
           correct_error_threshold(_thwFS, _theFS, c);
           break;
       case Counter::MemoryFree:
           if(t > _maxMem) t = _maxMem;
           _thwMem = (_maxMem - t)/_maxMem;
           correct_error_threshold(_thwMem, _theMem, c);
           break;
       case Counter::MemoryUsed:
            if(t > _maxMem) t = _maxMem;
           _thwMem = t/_maxMem;
           correct_error_threshold(_thwMem, _theMem, c);
           break;
       case Counter::Network:
           if(t> _maxNetwork) t = _maxNetwork;
           _thwNetwork = t/_maxNetwork;
           correct_error_threshold(_thwNetwork, _theNetwork, c);
           break;
       }
   }
   else
   {
       if(t > 1)
           t = 1;
       switch (c)
       {
       case Counter::CPU:
           _thwCPU = t;
           correct_error_threshold(_thwCPU, _theCPU, c);
           break;
       case Counter::PerCPU:
           _thwPerCPU = t;
           correct_error_threshold(_thwPerCPU, _thePerCPU, c);
           break;
       case Counter::FileSystemFree:
           _thwFS = (1 - t);
           correct_error_threshold(_thwFS, _theFS, c);
           break;
       case Counter::FileSystemUsed:
           _thwFS =  t;
           correct_error_threshold(_thwFS, _theFS, c);
           break;
       case Counter::MemoryFree:
           _thwMem = (1 - t)/_maxMem;
           correct_error_threshold(_thwMem, _theMem, c);
           break;
       case Counter::MemoryUsed:
           _thwMem = t;
           correct_error_threshold(_thwMem, _theMem, c);
           break;
       case Counter::Network:
           _thwNetwork = t;
           correct_error_threshold(_thwNetwork, _theNetwork, c);
           break;
       }
   }
}


void SysMonitor::SetErrorThreshold(double t, SysMonitor::Counter c, bool bAbsolute)
{
   QString s = get_section_string(c, Error);
   if(t<0)
   {
       t=0;
   }
   if(bAbsolute)
   {
       // error value must be not less than warning value
       switch (c)
       {
       case Counter::CPU:
           if( t > _maxCPU)
           {
               t = _maxCPU;
           }
           _theCPU = t;
           correct_warning_threshold(_thwCPU, _theCPU, c);
           break;
       case Counter::PerCPU:
           if( t > _maxPerCPU)
           {
               t = _maxPerCPU;
           }
           _thePerCPU = t;
           correct_warning_threshold(_thwPerCPU, _thePerCPU, c);
           break;
       case Counter::FileSystemFree:
           if(t > _maxFS) t = _maxFS;
           _theFS = (_maxFS - t)/_maxFS;
           correct_warning_threshold(_thwFS, _theFS, c);
           break;
       case Counter::FileSystemUsed:
           if(t > _maxFS) t = _maxFS;
           _theFS =  t/_maxFS;
           correct_warning_threshold(_thwFS, _theFS, c);
           break;
       case Counter::MemoryFree:
           if(t > _maxMem) t = _maxMem;
           _thwMem = (_maxMem - t)/_maxMem;
           correct_warning_threshold(_thwMem, _theMem, c);
           break;
       case Counter::MemoryUsed:
            if(t > _maxMem) t = _maxMem;
           _thwMem = t/_maxMem;
           correct_warning_threshold(_thwMem, _theMem, c);
           break;
       case Counter::Network:
           if(t> _maxNetwork) t = _maxNetwork;
           _thwNetwork = t/_maxNetwork;
           correct_warning_threshold(_thwNetwork, _theNetwork, c);
           break;
       }
   }
   else
   {
       if(t > 1)
           t = 1;
       switch (c)
       {
       case Counter::CPU:
           _theCPU = t;
           correct_warning_threshold(_thwCPU, _theCPU, c);
           break;
       case Counter::PerCPU:
           _thePerCPU = t;
           correct_warning_threshold(_thwPerCPU, _thePerCPU, c);
           break;
       case Counter::FileSystemFree:
           _theFS = (1 - t);
           correct_warning_threshold(_thwFS, _theFS, c);
           break;
       case Counter::FileSystemUsed:
           _theFS =  t;
           correct_warning_threshold(_thwFS, _theFS, c);
           break;
       case Counter::MemoryFree:
           _theMem = (1 - t)/_maxMem;
           correct_warning_threshold(_thwMem, _theMem, c);
           break;
       case Counter::MemoryUsed:
           _theMem = t;
           correct_warning_threshold(_thwMem, _theMem, c);
           break;
       case Counter::Network:
           _theNetwork = t;
           correct_warning_threshold(_thwNetwork, _theNetwork, c);
           break;
       }
   }
}

void SysMonitor::getFileSystemFree(const char *fsPart, double *absValue, double *relValue, double *total)
{
    if(!absValue && !relValue && !total)
        return;

     GetDiskFreeSpaceExA(disk.DirName,reinterpret_cast<PULARGE_INTEGER>(&disk.i64FreeBytesToCaller),
                           reinterpret_cast<PULARGE_INTEGER>(&disk.i64TotalBytes),
                           reinterpret_cast<PULARGE_INTEGER>(&disk.i64FreeBytes));

     double avail = disk.i64FreeBytes;
     setFreeSpace(disk.i64FreeBytes);
     double t = disk.i64TotalBytes;


     if(relValue)
     {
         *relValue = avail/t;
     }
       if(absValue)
      {
          *absValue = avail;
      }
       if(total)
       {
           *total = t;
       }
}

void SysMonitor::getFileSystemUsed(const char *fsPart, double *absValue, double *relValue, double *total)
{
   double t, *pt;
   pt = total ? total : &t;
   getFileSystemFree(fsPart, absValue, relValue, pt);
   if(absValue)
   {
       *absValue = *pt - *absValue;
   }
   if(relValue)
   {
       *relValue = 1.0 - *relValue;
   }

}

void SysMonitor::Start(int period)
{
  _timerID = startTimer(period);
  _bMonitoring = true;
}

void SysMonitor::Stop()
{
    ClearInstances();
    killTimer(_timerID);
    _bMonitoring = false;

}

void SysMonitor::Measure()
{
    double cpuValue;
    getCPULoad(&cpuValue);
    _cpuState = update_state(CPU, _thwCPU, _theCPU, cpuValue, _cpuState);

    double fsValue;
    getFileSystemUsed(_path,  nullptr, &fsValue, nullptr);
    _fsState = update_state(FileSystemUsed, _thwFS, _theFS, fsValue, _fsState);

    double memValue;
    getMemoryUsed(nullptr,&memValue, nullptr);
    _memState = update_state(MemoryUsed, _thwMem, _theMem, memValue, _memState);

    double networkValue;
    getNetworkLoad(nullptr, &networkValue,nullptr);
    _networkState = update_state(Network, _thwNetwork, _theNetwork, networkValue/100, _networkState);

    std::vector<double> cpuPerValue;
    getPerCPULoad(&cpuPerValue);
    _percpuState = update_state_(PerCPU, _thwPerCPU, _thePerCPU, cpuPerValue, _percpuState);

}


SysMonitor::SysMonitor()
{
}

double SysMonitor::getWarning(SysMonitor::Counter c, double defaultValue)
{
       return getThresholdValue(c, Warning, defaultValue);
}

double SysMonitor::getError(SysMonitor::Counter c, double defaultValue)
{
    return getThresholdValue(c, Error, defaultValue);
}

bool SysMonitor::isMonitoring()
{
    return _bMonitoring;
}

void SysMonitor::NotifyAlways(bool b)
{
    _bNotifyStateOnly = !b;
}

QString SysMonitor::get_section_string(SysMonitor::Counter c, const char *severity)
{
    QString s = "";
    switch (c)
    {
    case Counter::CPU:
        s = CPUSection;
        break;
    case Counter::PerCPU:
        s = PerCPUSection;
        break;
    case Counter::FileSystemFree:
    case Counter::FileSystemUsed:
        s = FileSystemSection;
        break;
    case Counter::MemoryFree:
    case Counter::MemoryUsed:
        s = MemorySection;
        break;
    case Counter::Network:
        s = NetworkSection;
        break;
    }
    s+=severity;
    return  s;
}


double SysMonitor::getThresholdValue(SysMonitor::Counter c, const char *severity, double defaultValue)
{
    QString s = get_section_string(c, severity);
    return 1.0;
}

SysMonitor::State SysMonitor::update_state_(Counter c, double warn, double err, std::vector<double> value, SysMonitor::State currState){


  for(int i =0;i<value.size();i++)
  {


  if(value[i]/100 >= err && err!=0 )
  {
    //std::cout<<"error threshold: "<<err*100<<"% "<<std::endl;
      if(!_bNotifyStateOnly || currState != Err)
      {
          emit error(c, value[i]/100);
          return Err;


      }

  }
  else if (value[i]/100  >= warn && warn!=0 )
  {
 //  std::cout<<"warning threshold: "<<warn*100<<"% "<<std::endl;

      if(!_bNotifyStateOnly || currState != Warn)
      {
      emit warning(c, value[i]/100);
      return Warn;
      }
  }
 /* else if(!_bNotifyStateOnly || currState!=Normal)
  {
     emit norm(c, value[i]/100);
     return Normal;

  }*/
  emit norm(c,value[i]/100);

  }
  return Normal;


}

unsigned long long SysMonitor::getFreeSpace() const
{
  return freeSpace;
}

void SysMonitor::setFreeSpace(unsigned long long value)
{
  freeSpace = value;
}



SysMonitor::State SysMonitor::update_state(Counter c, double warn, double err, double value, SysMonitor::State currState)
{
  if(value >= err && err!=0)
    {
    //  std::cout<<"error threshold: "<<err*100<<"% "<<std::endl;

        if(!_bNotifyStateOnly || currState != Err)
        {
            emit error(c, value);
            return Err;
        }
    }
    else if (value >= warn && warn!=0  )
    {
    //  std::cout<<"warning threshold : "<<warn*100<<"% "<<std::endl;

        if(!_bNotifyStateOnly || currState != Warn)
        {
        emit warning(c, value);
        return Warn;
        }
    }
    else if(!_bNotifyStateOnly || currState!=Normal)
    {
       emit norm(c, value);
       return Normal;
    }
    return currState;
}

void SysMonitor::timerEvent(QTimerEvent *)
{
    Measure();
}
