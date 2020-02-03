#ifndef SYSMON_H
#define SYSMON_H
//sysmonitor
#include"sysmonitor.h"
//3rd party includes
#include <QApplication>
#include "qnamespace.h"
#include <QStandardItemModel>
#include "Query.h"
#include <QObject>


/*! SysMon class which can be called outside to start monitoring with pre-defined(RTSS.ini) parameters. Platform dependent (Windows).*/

class SysMon:public QObject
{
  Q_OBJECT

public:

  SysMon( QObject *parent = 0);

  //Init COM library
   void init(void);

  ~SysMon();
   //triggers sysmonitor class in pre-defined interval and passes values to it
  void startRun(bool state, double thresholdsFromConfig[], std::vector<int> &watchCPUFromConfig, string &filePathFromConfig,
                std::vector<int> &netwAdpFromConfig , double &esImSize,int &freeSpace);
  //stops system monitoring
  void exit();
  //use absolute values
  void UseAbsolute();
  //checks whether there is enough place or not for incoming image acquisation
  bool SpaceForAnIncomingImage(int numberOfFrames);
  //checks whether reserved space is exceeded or not
  bool reservedFreeSpace();
  //setter and getter for interval
  int getInterval() const;
  void setInterval(int value);
  //set estimated image size
  void setEsImageSize(double value);

  int getReservFreeSpace() const;
  void setReservFreeSpace(int &value);

public slots:
  void on_norm(SysMonitor::Counter c, double value);
  void on_warning(SysMonitor::Counter c, double value);
  void on_error(SysMonitor::Counter c, double value);

private:
  SysMonitor* _sysMonitor;
  std::map<int,string> processNumbers;
  int interval;
  double esImageSize;
  void updateUI();
  double _maxMemory, _maxFS, _maxNetwork;
  bool _bAbsolute;
  int reservFreeSpace;



};
#endif // RUN_H
