#ifndef PERFHELPER_H
#define PERFHELPER_H
//3rd party includes
#include<QApplication>
#include <Pdh.h>
#include<QObject>
#include<qdebug.h>
#include<iostream>
#include <algorithm>
#include <iterator>
//query
#include"query.h"

/*! Helper class for SysMonitor. Measures network performance. Platform dependent (Windows).*/


class NetworkPerfHelper
{
   // struct declaration
  struct CounterData {
          PDH_HCOUNTER hcounter;
          std::string  name;
          std::string  path;
      };
public:
  /// Constructor
  NetworkPerfHelper();
  NetworkPerfHelper(const Query&) = delete;
  NetworkPerfHelper(NetworkPerfHelper&& src) : hquery(src.hquery) {
    src.hquery = 0;
    // std::move these if you want
    // this is really just for keeping hquery unique
    counter_list = src.counter_list;
    object_name = src.object_name;
    instance_name = src.instance_name;
  }
  NetworkPerfHelper& operator = (const NetworkPerfHelper&) = delete;
  NetworkPerfHelper& operator = (NetworkPerfHelper&& src) {
    if (hquery) PdhCloseQuery(hquery);
    hquery = src.hquery;
    src.hquery = 0;
    counter_list = src.counter_list;
    object_name = src.object_name;
    instance_name = src.instance_name;
    return *this;
  }

  operator PDH_HQUERY () const { return hquery; }
  bool IsOk() const { return !!hquery; }
  operator const void* () const { return (const void*)IsOk(); }

  void SetInstance(const std::string& name);


  void SetObject(const std::string& name);


  /// Destructor
  ~NetworkPerfHelper(){
    if (!hquery) return;
    PdhCloseQuery(hquery);

  }

  void AddCounter() {}
  template <typename ...more>

  void AddCounter(const std::string& name, more... args);



  /// Get current speed and bandwidth of network interface.
  /// \param [in,out] speed Receive current network speed. May be NULL.
  /// \param [in, out] bandwidth Receive network bandwidth. May be NULL.
  void GetNetworkLoad(double* speed, double* bandwidth);



  void setIndexOfAdp(const std::vector<int> &adp);

private:
  PDH_HQUERY               hquery;
  PDH_HQUERY _phQuery;
  PDH_HCOUNTER _phCounterBandWidth;
  PDH_HCOUNTER _phCounterTotal;
  std::vector<CounterData> counter_list;
  std::string              object_name;
  std::string              instance_name;
  std::vector<int> indexOfAdp;
  std::string adapterName;
  Query query;




  bool _bValid;
  double _bandwidth, _speed;
};
/*! Helper class for SysMonitor. Measures CPU performance. Platform dependent (Windows).*/
class CPUPerfHelper
{
public:
    /// Constructor
    CPUPerfHelper();
    /// Returns current CPU load.
    double GetCPULoad();

private:
    double doubleFromFILETIME(FILETIME *ft);
};

/*! Helper class for SysMonitor. Measures memory load. Platform dependent (Windows).*/
class MemoryPerfHelper

{
public:
    /// Constructor
    MemoryPerfHelper();
    /// Returns used memory in bytes.
    double GetMemoryLoad();
    /// Returns total memory in bytes.
    double GetMemoryTotal();
};
/*!Helper class for SysMonitor. Measures per CPU load based on user's selection from RTSS.ini file.Platform dependent (Windows). */
class PerCPUPerfHelper {

public:
    //Constructor
    PerCPUPerfHelper();
    // Returns choosen CPUs' load
    std::vector<double> GetPerCpuLoad();

    //getter and setter for vector which holds checked comboBoxes' index
    std::vector<int> v() const;
    void setV(const std::vector<int> &v);

private:
    std::vector<int> v_;
    int vectorSize;

};


#endif // PERFHELPER_H
