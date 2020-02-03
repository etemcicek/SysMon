//me
#include "sysmon.h"

//3rd party includes
#include <comdef.h>
#include<iostream>
#include<qdebug.h>
#include<QSignalMapper>
//CLS
#include <oraCentralLoggingService.h>

SysMon::SysMon(QObject *parent) : QObject(parent)
{

  init();
  reservFreeSpace=0;
  _sysMonitor = SysMonitor::getMonitor();

  setInterval(0);

  connect(_sysMonitor, SIGNAL(norm(SysMonitor::Counter,double)), this, SLOT(on_norm(SysMonitor::Counter,double)));
  connect(_sysMonitor, SIGNAL(warning(SysMonitor::Counter,double)), this, SLOT(on_warning(SysMonitor::Counter,double)));
  connect(_sysMonitor, SIGNAL(error(SysMonitor::Counter,double)), this, SLOT(on_error(SysMonitor::Counter,double)));



  processNumbers[1]="CPU";
  processNumbers[2]="File system";
  processNumbers[4]="File system used";
  processNumbers[6]="Per CPU";
  processNumbers[8]="Memory Used";
  processNumbers[0x10]="Memory Free";
  processNumbers[0x20]="Network Adapters";

  LOG_INFO(QString("SysMon:SysMon() successfully initialized."));

}

void SysMon::init()
{
  CoUninitialize();

  HRESULT hres;

  hres = CoInitializeEx(0, COINIT_MULTITHREADED);


  // Step 1: --------------------------------------------------
  // Initialize COM. ------------------------------------------
  //CoInitialize(NULL);


  if (FAILED(hres))
  {
    std::cout << "Failed to initialize COM library. Error code = 0x" << hex << hres << endl;
    return ;                  // Program has failed.
  }

  // Step 2: --------------------------------------------------
  // Set general COM security levels --------------------------
  // Note: If you are using Windows 2000, you need to specify -
  // the default authentication credentials for a user by using
  // a SOLE_AUTHENTICATION_LIST structure in the pAuthList ----
  // parameter of CoInitializeSecurity ------------------------

  hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
        );


  if (FAILED(hres))
  {
    std:: cout << "Failed to initialize security. Error code = 0x"
               << hex << hres << endl;
    CoUninitialize();
    return ;                    // Program has failed.
  }

}
SysMon::~SysMon(){

  _sysMonitor->Stop();
}



void SysMon::UseAbsolute()
{
  updateUI();
  if(!_sysMonitor->isMonitoring())
    _sysMonitor->Measure();
}

bool SysMon::SpaceForAnIncomingImage(int numberOfFrames)
{
  if((numberOfFrames*esImageSize*1024*1024)>(_sysMonitor->getFreeSpace()))
  {
    return true;
  }
  else
  {
    return false;
  }

}

bool SysMon::reservedFreeSpace()
{


  if((getReservFreeSpace())>(_sysMonitor->getFreeSpace()/1024/1024/1024))
  {

    LOG_ERROR(tr("SysMon::reservedFreeSpace(): Reserved space is %1 GB, "
                 "But free space is %2 GB !! ").arg(getReservFreeSpace()).arg(_sysMonitor->getFreeSpace()/1024/1024/1024));
    return false;
  }
  else
  {
    LOG_INFO(tr("SysMon::reservedFreeSpace(): Available space is more than %1 GB").arg(getReservFreeSpace()));
    return true;
  }

}

void SysMon::startRun(bool state, double thresholdsFromConfig[], std::vector<int> &watchCPUFromConfig,
                      std::string &filePathFromConfig,std::vector<int> &netwAdpFromConfig,double &esImSize,int &freeSpace)
{
  if(state)
  {
    _sysMonitor->initialize(thresholdsFromConfig,filePathFromConfig);
    setEsImageSize(esImSize);
    setReservFreeSpace(freeSpace);
    _sysMonitor->SetIndexOfAdapters(netwAdpFromConfig);   //setter for a vector indexes which switch current adapter
    _sysMonitor->SetAlertCores(watchCPUFromConfig);        //setter for a vector which holds checkboxes' indexes

    _sysMonitor->Start(interval);
  }
  else
  {
    _sysMonitor->Stop();

  }

}


void SysMon::exit(){

  _sysMonitor->Stop();
}

void SysMon::updateUI(){
  _bAbsolute=true;

  if(!_bAbsolute)
  {
    _maxNetwork = 100.0;
    _maxFS = 100.0;
    _maxMemory = 100.0;
  }
  double valueW, valueE, valueMax = 100.0;

  valueW =_sysMonitor->getWarning(SysMonitor::CPU, 0.6);
  valueE =_sysMonitor->getError(SysMonitor::CPU,0.95);
  valueW =_sysMonitor->getWarning(SysMonitor::PerCPU, 0.6);
  valueE =_sysMonitor->getError(SysMonitor::PerCPU,0.95);

  valueW =_sysMonitor->getWarning(SysMonitor::FileSystemUsed, 0.8);
  valueE =_sysMonitor->getError(SysMonitor::FileSystemUsed, 0.9);
  valueMax = 100.0;

  if(_bAbsolute)
  {
    _sysMonitor->getFileSystemUsed(nullptr,nullptr,nullptr,&valueMax);
    valueMax = valueMax/0x40000000;
  }
  _maxFS = valueMax;


  valueW =_sysMonitor->getWarning(SysMonitor::MemoryUsed, 0.75);
  valueE =_sysMonitor->getError(SysMonitor::MemoryUsed, 0.9);
  valueMax = 100.0;
  if(_bAbsolute)
  {
    _sysMonitor->getMemoryUsed(nullptr,nullptr,&valueMax);
    valueMax = valueMax/0x40000000;
  }
  _maxMemory = valueMax;
  valueW =_sysMonitor->getWarning(SysMonitor::Network, 0.9);
  valueE =_sysMonitor->getError(SysMonitor::Network, 0.98);
  valueMax = 100.0;
  if(_bAbsolute)
  {
    _sysMonitor->getNetworkLoad(nullptr,nullptr,&valueMax);
    valueMax = valueMax/1000;// Kbytes/s
  }
  _maxNetwork = valueMax;

}

int SysMon::getReservFreeSpace() const
{
  return reservFreeSpace;
}

void SysMon::setReservFreeSpace(int &value)
{
  reservFreeSpace = value;
}

void SysMon::on_norm(SysMonitor::Counter c, double value)
{
  //std::cout<<"Counter "<<c<<" value : "<<value*100<<" is normal"<<std::endl;
  // std::cout<<"\n "<<std::endl;
}

void SysMon::on_warning(SysMonitor::Counter c, double value)
{

  LOG_WARN(tr("SysMon::on_Warning() %1 value : %2 ").arg(QString::fromStdString(processNumbers[c])).arg(value));

}

void SysMon::on_error(SysMonitor::Counter c, double value)
{

  LOG_ERROR(tr("SysMon::on_Error() %1 value : %2 ").arg(QString::fromStdString(processNumbers[c])).arg(value*100));
}

void SysMon::setEsImageSize(double value)
{
  esImageSize = value;
}

int SysMon::getInterval() const
{
  return interval;
}

void SysMon::setInterval(int value)
{
  interval = value;
}




