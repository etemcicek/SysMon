#include "config.h"
#include<qdebug.h>
#include <QApplication>


void Config::bind_file(const QString& path)
{
    _settings = new QSettings(path, QSettings::IniFormat);
}

Config::Config():_settings(nullptr)
{
    QString path = QApplication::applicationDirPath() + "/SM.config";
    bind_file(path);
}

Config::~Config()
{
  save();
  delete _settings;
}

Config::Config(const char *path)
{
    bind_file(path);
}

double Config::setValue(const char *key, double value)
{
    if(_settings)
    {
      _settings->setValue(key, value);
      _dirty = true;
    }
    return value;
}

void Config::save()
{
    if(_settings && _dirty)
    {
        _settings->sync();
    }
}

double Config::getValue(const char* key, double defaultValue)
{
    if(_settings)
    {
      return  _settings->value(key, defaultValue).toDouble();
    }
    return  defaultValue;
}


