#define _WIN32_DCOM
//me
#include "perfhelper.h"
//3rd party includes
#include <PdhMsg.h>
#include <windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <iostream>
#include<qdebug.h>
#include <iomanip>

# pragma comment(lib, "wbemuuid.lib")

using namespace std;
NetworkPerfHelper::NetworkPerfHelper()
{

  PdhOpenQuery(0, 0, &hquery) == ERROR_SUCCESS || (hquery = 0);

  std::vector<std::string> getNetworkInstances;
  getNetworkInstances=query.SetObject("Network Interface",0);

  for(int i=0;i<getNetworkInstances.size();i++)
  {
    indexOfAdp.push_back(0);
  }

}

void NetworkPerfHelper::SetInstance(const string &name)
{
  instance_name = name;

}

void NetworkPerfHelper::SetObject(const string &name)
{
  object_name = name;
  auto counter_names = ListCounters(name);
  if (counter_names.instances.size())
  {

    for(int i=0;i<indexOfAdp.size();i++)
    {

      if(indexOfAdp[i]==1)
      {

        SetInstance(counter_names.instances[i]);
      }

    }

  }
}



void NetworkPerfHelper::GetNetworkLoad(double *speed, double *bandwidth)
{

  SetObject("Network Interface");
  AddCounter("Current Bandwidth");
  AddCounter("Bytes Total/sec");


  size_t max_name_len = 0;
  for (const auto& counter : counter_list)
  {
    if (counter.name.length() > max_name_len) max_name_len = counter.name.length();
  }
  auto status = PdhCollectQueryData(hquery);
  if (status != ERROR_SUCCESS)
  {
    // std::cout << "CounterPollingDump: PdhCollectQueryData failed: " << std::hex << status << '\n';
    return;
  }
  // std::cout << "      =======================\n";



  if(_bValid)
  {
    DWORD counter_type;
    PDH_FMT_COUNTERVALUE fmt_value1,fmt_value2;
    if(bandwidth){

      auto status1 = PdhGetFormattedCounterValue(counter_list[0].hcounter, PDH_FMT_DOUBLE, &counter_type, &fmt_value1);
      if (status1 != ERROR_SUCCESS)
      {
        if (status1 == PDH_INVALID_DATA)
        {
          std::cout << " -- no data --\n";
        }
        //  std::cout << "CounterPollingDump: PdhGetFormattedCounterValue failed: " << std::hex << status1 << '\n';
        return;
      }

      *bandwidth=fmt_value1.doubleValue;
      //   std::cout << "  " << fmt_value1.doubleValue << '\n';

      if(fmt_value1.CStatus==PDH_CSTATUS_NEW_DATA)
      {
        _bandwidth=fmt_value1.doubleValue;

      }
    }
    if(speed){

      auto status1 = PdhGetFormattedCounterValue(counter_list[1].hcounter, PDH_FMT_DOUBLE, &counter_type, &fmt_value2);
      if (status1 != ERROR_SUCCESS)
      {
        if (status1 == PDH_INVALID_DATA)
        {
          //  std::cout << " -- no data --\n";
        }
        //     std::cout << "CounterPollingDump: PdhGetFormattedCounterValue failed: " << std::hex << status1 << '\n';
        return;
      }
      *speed=fmt_value2.doubleValue;

      if(fmt_value2.CStatus==PDH_CSTATUS_NEW_DATA)
      {
        _speed=fmt_value2.doubleValue;
      }

    }

  }

}

void NetworkPerfHelper::setIndexOfAdp(const std::vector<int> &adp)
{
  for(int i=0;i<adp.size();i++)
  {
    indexOfAdp[i]=adp[i];
  }
  //For adapters to be processed with its name, just comment this out
  // adapterName=adp;
}

CPUPerfHelper::CPUPerfHelper()
{

}

double CPUPerfHelper::GetCPULoad()
{
  FILETIME it, kt, ut;
  FILETIME it1, kt1, ut1;
  double ld = 0.0;
  if(GetSystemTimes(&it, &kt, &ut))
  {
    Sleep(250);
    if(GetSystemTimes(&it1, &kt1, &ut1))
    {
      double idleTime = doubleFromFILETIME(&it1) - doubleFromFILETIME(&it);
      double kernelTime = doubleFromFILETIME(&kt1) - doubleFromFILETIME(&kt);
      double userTime = doubleFromFILETIME(&ut1) - doubleFromFILETIME(&ut);
      ld = 1 - idleTime/(kernelTime + userTime);
    }
  }
  return ld;
}


double CPUPerfHelper::doubleFromFILETIME(FILETIME *ft)
{

  ULARGE_INTEGER uli = { {0, 0} };
  uli.LowPart = ft->dwLowDateTime;
  uli.HighPart = ft->dwHighDateTime;
  return (double)uli.QuadPart;
}

MemoryPerfHelper::MemoryPerfHelper()
{

}

double MemoryPerfHelper::GetMemoryLoad()
{
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof (MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&mem);
  //   unsigned long long um = mem.ullTotalPhys - mem.ullAvailPhys;
  return (double)(mem.ullTotalPhys - mem.ullAvailPhys);
}

double MemoryPerfHelper::GetMemoryTotal()
{
  MEMORYSTATUSEX mem;
  mem.dwLength = sizeof (MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&mem);
  return (double)mem.ullTotalPhys;
}

PerCPUPerfHelper::PerCPUPerfHelper()
{
  SYSTEM_INFO systemInfo;
  GetSystemInfo( &systemInfo );
  vectorSize=systemInfo.dwNumberOfProcessors;
  for(int i=0;i<vectorSize;i++)
  {

    v_.push_back(0);
  }


}

std::vector<double> PerCPUPerfHelper::GetPerCpuLoad()
{
  HRESULT hres;


  // Step 3: ---------------------------------------------------
  // Obtain the initial locator to WMI -------------------------

  IWbemLocator *pLoc = NULL;

  hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID *)&pLoc);

  if (FAILED(hres))
  {
    cout << "Failed to create IWbemLocator object."
         << " Err code = 0x"
         << hex << hres << endl;
    CoUninitialize();
    // Program has failed.
  }

  // Step 4: -----------------------------------------------------
  // Connect to WMI through the IWbemLocator::ConnectServer method

  IWbemServices *pSvc = NULL;

  // Connect to the root\cimv2 namespace with
  // the current user and obtain pointer pSvc
  // to make IWbemServices calls.
  hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (e.g. Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
        );

  if (FAILED(hres)) {
    cout << "Could not connect. Error code = 0x" << hex << hres << endl;
    pLoc->Release();
    CoUninitialize();
    // Program has failed.
  }

  // cout << "Connected to ROOT\\CIMV2 WMI namespace" << endl;


  // Step 5: --------------------------------------------------
  // Set security levels on the proxy -------------------------

  hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
        );

  if (FAILED(hres))
  {
    cout << "Could not set proxy blanket. Error code = 0x"
         << hex << hres << endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    // Program has failed.
  }

  // Step 6: --------------------------------------------------
  // Use the IWbemServices pointer to make requests of WMI ----

  // For example, get the name of the operating system

  IEnumWbemClassObject* pEnumerator = NULL;
  IWbemClassObject *pclsObj;
  int i;

  int clicked=0;
  std::vector<double> PCPUIndexes;
  for(int x=0;x<v_.size();x++)
  {
    if(v_[x]==1){
      PCPUIndexes.push_back(0.0);
    }
  }


  i = 0;
  hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_PerfFormattedData_PerfOS_Processor"),

        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

  if (FAILED(hres))
  {
    cout << "Query for operating system name failed."
         << " Error code = 0x"
         << hex << hres << endl;
    pSvc->Release();
    pLoc->Release();
    CoUninitialize();
    // Program has failed.
  }

  // Step 7: -------------------------------------------------
  // Get the data from the query in step 6 -------------------


  ULONG uReturn = 0;


  while (pEnumerator) {
    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
                                   &pclsObj, &uReturn);

    if (0 == uReturn)
    {
      break;
    }

    VARIANT vtProp;

    // Get the value of the Name property
    hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
    hr = pclsObj->Get(L"PercentProcessorTime", 0, &vtProp, 0, 0);
    //  wcout << " CPU Usage of CPU " << i << " : " << vtProp.bstrVal << endl;


    if(i!=vectorSize )
    {
      if(v_[i] == 1 && i!=vectorSize+1  )
      {

        PCPUIndexes[clicked]=_wtoi(vtProp.bstrVal);

        //   a+=_wtoi(vtProp.bstrVal);
        clicked++;
      }
    }

    //IMPORTANT!!
    pclsObj->Release();

    i++;
  }


  // Cleanup
  // ========


  return PCPUIndexes;


}

std::vector<int> PerCPUPerfHelper::v() const
{
  return v_;
}

void PerCPUPerfHelper::setV(const std::vector<int> &v)
{
  for(int i=0;i<v.size();i++)
  {

    for(int m=0;m<v_.size();m++)
    {

      if(v[i]==m)
      {
        v_[m]=1;
      }
      else
      {
        continue;
      }

    }

  }

}

template <typename ...more>
void NetworkPerfHelper::AddCounter(const string &name, more... args)
{
  if (!hquery)
  {
    std::cout << "AddCounter: Query was not successfully created\n";
    return;
  }
  if (object_name.empty())
  {
    return;
  }
  if (instance_name.empty())
  {
    return;
  }

  CounterData counter_data;
  counter_data.name = name;
  counter_data.path = CounterPath(object_name, name, instance_name);
  auto status = PdhAddCounterA(hquery, counter_data.path.c_str(), 0, &counter_data.hcounter);
  if (status != ERROR_SUCCESS)
  {
    std::cout << "AddCounter Failed: " << std::hex << status << '\n';
    return;
  }
  counter_list.push_back(counter_data);
  AddCounter(args...);
}
