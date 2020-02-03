#ifndef SYSMONITOR_H
#define SYSMONITOR_H

//3rd party includes
#include <Windows.h>
#include <string>
#include <QObject>
#include<iostream>
#include<vector>

#define PATH_MAX 260


class QTimer;
using namespace std;
//struct declaration
struct diskdata
{
    LPCSTR  DirName;
    unsigned __int64    i64FreeBytesToCaller;
    unsigned __int64    i64TotalBytes;
    unsigned __int64    i64FreeBytes;
};
/*!
The class defines the functions for the platform specific implementations of SysMonitor.

SysMonitor defines six functions for obtaining four types of information about system performance.
- getCPULoad() retrievs immediate information about loading of CPU.
- getNetworkLoad() get information about network interface traffic.
- getFileSystemFree() and getFileSystemUsed() get information about file system storage device. Use setFSPath() to reference folder which you want monitoring.
- getMemoryFree() and getMemoryUsed() get system memory usage information.
Enumeration Counter contains values defined for each type of performance.

Class contains three types of signals for notifying about changes of system parameters.  It reads the system information with certain period of time. Notifications will be sent with every period or only in case when measured parameter crossed defined threshold value. This behaviour may be changed with NotifyAlways() method. Signals may be forced to fire once with call Measure() method.

There are two type of thresholds: warning and error. The threshold values can be adjusted by calling SetWarningThreshold() and/or SetErrorThreshold(). Values persist in the configuration file.

The class has no public constructors. To obtain an instance of platform dependent inherited class use static method getMonitor();

\warning Call initialize() before any other non-static method call!

*/

class SysMonitor: public QObject
{
    Q_OBJECT
public:
  static  const char  CPUSection[] ;
  static  const char  PerCPUSection[] ;
  static  const char  MemorySection[] ;
  static  const char  NetworkSection[] ;
  static  const char  FileSystemSection[] ;
  static  const char  Error[];
  static  const char  Warning[] ;

    //! The only way to get instance of platform dependent inherited class.
    static SysMonitor* getMonitor();
    //!Destroys the SysMonitor and the contained object.
    virtual ~SysMonitor();
    /*! When overloaded in derived class provides initialization of all perfomance measuring objects. May be time consuming, so not called from the constructor. Must be the first method to call after instance creation!*/
    virtual void initialize(double thresholdsConfig[],std::string &filePathFromConfig)=0;
    /*!
      This enum defines the types of performance counters
    */
    inline  void setDiskDirectory(LPCSTR directoryname){
          disk.DirName = directoryname;}
    enum Counter
    {
        //! Processor unit counter
        CPU = 1,
        //! Available part of file system
        FileSystemFree = 2,
        //! Used part of filesystem
        FileSystemUsed = 4,
        PerCPU=6,
        //! Used memory
        MemoryUsed = 8,
        //! Free memory
        MemoryFree = 0x10,
        //! Network traffic counter
        Network = 0x20
    };
    /*! Sets the value which will be used as threshold for emitting warning() or norm() signals. The value will persist in the configuration file.
     * \param [in] t The value of threshold to set.
     * \param [in] c Counter type for which threshold will be set.
     * \param [in] bAbsolute Optional. If bAbsolute is true, the parameter 't' means absolute (not relative) value.
     */
    void SetWarningThreshold(double t, Counter c, bool bAbsolute = false);


    /*! Sets the value which will be used as threshold for emitting error() or warning() signals. The value will persist in the configuration file.
     * \param [in] t The value of threshold to set.
     * \param [in] c Counter type for which threshold will be set.
     * \param [in] bAbsolute Optional. If bAbsolute is true, the parameter 't' means absolute (not relative) value.
     */
    void SetErrorThreshold(double t, Counter c, bool bAbsolute = false);
    /*! When overloaded in derived class obtains the value of current CPU load.
     * Usage:
     *
     * \code{.cpp}
     * double value;
     * getCPULoad(&value);
     * \endcode
    */
    virtual void getCPULoad(double* value)=0;
    /*! When overloaded in derived class obtains the value of current Per CPU load.
     * Usage:
     *
     * \code{.cpp}
     * double value;
     * getPerCPULoad(&value);
     * \endcode
    */
    virtual void getPerCPULoad(std::vector<double>* value)=0;

    /*! When overloaded in derived class sets the value of currently checked checkboxes .
     * Usage:
     *
     * \code{.cpp}
     * std::vector<int> vect;
     * SetAlertCores(vect);
     * \endcode
    */
    virtual void SetAlertCores(std::vector<int> vect)=0;

    /*! When overloaded in derived class obtains values for the network loading.
     * Usage:
     *
     * \code{.cpp}
     * double abs_value, bandWidth;
     * //It is possible not to provide all of the parameters
     * getNetworkLoad(&abs_value, 0, &bandWidth);
     * \endcode
     *
     * \param [in,out] absValue Pointer to variable for current absolute value of network loading. May be NULL.
     * \param [in,out] relValue Pointer to variable for current relative value of network loading. May be NULL.
     * \param [in,out] bandWidth Pointer to variable for value of network bandwidth. May be NULL.
     *
     * NULL parameters will be ignored.
    */

    virtual void getNetworkLoad(double* absValue, double* relValue, double* bandWidth)=0;

    /*! When overloaded in derived class clears object's initial values .
     * Usage:
     *
     * \code{.cpp}
     * ClearInstances();
     * \endcode
    */

    virtual void ClearInstances()=0;


    /*! When overloaded in derived class sets the value of currently chosen Adapter .
     * Usage:
     *
     * \code{.cpp}
     * int adp;;
     * SetIndexOfAdapters(adp);
     * \endcode
    */
    virtual void SetIndexOfAdapters(std::vector<int> &adp)=0;
    /*! Obtains values for the filesystem free space.
     * Usage:
     *
     * \code{.cpp}
     * double abs_value, total;
     * //It is possible not to provide all of the parameters
     * getFileSystemFree(&abs_value, 0, &total);
     * \endcode
     *
     * \param [in] fsPart Path to folder of interest. If NULL then application folder will be used.
     * \param [in,out] absValue Pointer to variable for current absolute value of free filesystem space in bytes. May be NULL.
     * \param [in,out] relValue Pointer to variable for current relative value of free filesystem space relative to total space. May be NULL.
     * \param [in,out] total Pointer to variable for value of total filesystem space. May be NULL.
     *
     * NULL parameters will be ignored.
    */

    virtual void getFileSystemFree(const char* fsPart, double* absValue, double* relValue, double *total);
    /*! Obtains values for the filesystem used space.
     * Usage:
     *
     * \code{.cpp}
     * double abs_value, total;
     * //It is possible not to provide all of the parameters
     * getFileSystemUsed(&abs_value, 0, &total);
     * \endcode
     *
     * \param [in] fsPart Path to folder of interest. If NULL then application folder will be used.
     * \param [in,out] absValue Pointer to variable for current absolute value of used filesystem space in bytes. May be NULL.
     * \param [in,out] relValue Pointer to variable for current relative value of used filesystem space relative to total space. May be NULL.
     * \param [in,out] total Pointer to variable for value of total filesystem space. May be NULL.
     *
     * NULL parameters will be ignored.
    */
    virtual void getFileSystemUsed(const char* fsPart, double* absValue, double* relValue, double *total);
    /*! When overloaded in derived class obtains values for the free memory.
     * Usage:
     *
     * \code{.cpp}
     * double relValue, total;
     * //It is possible not to provide all of the parameters
     * getMemoryFree(NULL, &relValue, &total);
     * \endcode
     *
     * \param [in,out] absValue Pointer to variable for current absolute value of free memory in bytes. May be NULL.
     * \param [in,out] relValue Pointer to variable for current value of free memory relative to total memory. May be NULL.
     * \param [in,out] total Pointer to variable for total value of memory. May be NULL.
     *
     * NULL parameters will be ignored.
    */
    virtual void getMemoryFree(double* absValue, double* relValue, double *total)=0;
    /*! When overloaded in derived class obtains values for the used memory.
     * Usage:
     *
     * \code{.cpp}
     * double relValue, total;
     * //It is possible not to provide all of the parameters
     * getMemoryUsed(NULL, &relValue, &total);
     * \endcode
     *
     * \param [in,out] absValue Pointer to variable for current absolute value of used memory in bytes. May be NULL.
     * \param [in,out] relValue Pointer to variable for current value of used memory relative to total memory. May be NULL.
     * \param [in,out] total Pointer to variable for total value of memory. May be NULL.
     *
     * NULL parameters will be ignored.
    */
    virtual void getMemoryUsed(double* absValue, double* relValue, double *total)=0;
    /*! Starts the periodic measuring of system parameters with provided period.*/
    void Start(int period);
    /*! Stops the periodic measuring of system parameters.*/
    void Stop();
    /*! Measures system parameters once. Causes the signals emitting */
    void Measure();
    /*! Reread threshold values from configuration file*/
    /*! Get current warning threshold
     * \param [in] c Perfomance counter for retriving value
     * \param [in] defaultValue Default value. Will be returned if there no such settings for counter.
     * \retval Warning threshold for provided counter.
     */
    double getWarning(Counter c, double defaultValue);
    /*! Get current error threshold
     * \param [in] c Perfomance counter for retriving value
     * \param [in] defaultValue Default value. Will be returned, if there no such settings for counter.
     * \retval Error threshold for provided counter.
     */
    double getError(Counter c, double defaultValue);

    /*!  Returns true if periodic measuring is started. */
    bool isMonitoring();
    /*! If parameter 'b' is true monitor will signal after each measuring. Else monitor will signal only when parameter value crossed the threshold.\n
        For example:\n
        Warning threshold is set on 0.5 relative value. Error is 0.9. Previous parameter value was 0.6.
        - Current parameter value 0.55 emits warning signal if NotifyAlways set to true, and emits nothing if NotifyAlways set to false.
        - Current parameter value 0.4 emits norm signal in either case.
        - Current parameter value 0.95 emits error signal in either case.
    */
    void NotifyAlways(bool b);

    //getter and setter for main drive's free space
    unsigned long long getFreeSpace() const;
    void setFreeSpace(unsigned long long value);

signals:
    /*! Emits norm signal \see NotifyAlways()*/
    void norm(SysMonitor::Counter c, double value);
    /*! Emits warning signal \see NotifyAlways()*/
    void warning(SysMonitor::Counter c, double value);
    /*! Emits error signal \see NotifyAlways()*/
    void error(SysMonitor::Counter c, double value);


protected:
    SysMonitor();

    /*! CPU warning threshold*/
    double  _thwCPU;
    /*! CPU error threshold*/
    double  _theCPU;
    /*! CPU maximum value*/
    double  _maxCPU;


    /*! CPU warning threshold*/
    double  _thwPerCPU;
    /*! CPU error threshold*/
    double  _thePerCPU;
    /*! CPU maximum value*/
    double  _maxPerCPU;

    /*! Network warning threshold*/
    double _thwNetwork;
    /*! Network error threshold*/
    double _theNetwork;
    /*! Network bandwidth*/
    double _maxNetwork;

    /*! Filesystem warning threshold. */
    double _thwFS;
    /*! Filesystem error threshold. */
    double _theFS;
    /*! Filesystem capacity. */
    double  _maxFS;

    /*! Memory warning threshold. */
    double _thwMem;
    /*! Memory error threshold. */
    double _theMem;
    /*! Memory total. */
    double _maxMem;
    /*! Path to folder in filesystem for measuring filesystem load.*/
    char _path[PATH_MAX];
    ///! Timer callback
    void timerEvent(QTimerEvent *event);
    std::vector<int> v;


private:
    enum State
    {
        Normal, Warn, Err
    };
    State _cpuState = Normal,_percpuState=Normal, _memState= Normal, _networkState= Normal, _fsState= Normal;
    SysMonitor (const SysMonitor& monitor):QObject (){(void)monitor;}
    SysMonitor& operator = (const SysMonitor& monitor){(void)monitor; return *this;}
    double getThresholdValue(Counter c, const char* severity, double defaultValue);
    QString get_section_string(SysMonitor::Counter c, const char *severity);
    void correct_error_threshold(double &warn, double &err, Counter c);
    void correct_warning_threshold(double &warn, double &err, Counter c);
    int _timerID;
    bool _bMonitoring = false;
    bool _bNotifyStateOnly = false;
    diskdata disk;
    State update_state(Counter c, double warn, double err, double value, State currState);
    State update_state_(Counter c, double warn, double err, std::vector<double> value, State currState);
    unsigned __int64 freeSpace;

};

#endif // SYSMONITOR_H
