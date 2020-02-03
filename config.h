#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <QSettings>
using namespace std;

/*!
Allows retrieving and storing platform-independent configuration settings in INI-like formatted file.

You can create the instance of Config by passing the path to the configuration file as a constructor parameter. If you  use default constructor with no parameters, Config will use file "SM.config" in the application folder.

All changes will be synchronized during object destruction and/or when calling save() method.

\code{.cpp}
    {
        //default config in the application folder
        Config config;

        //retrieving value
        double e = config.getValue("Extent", 10.0);

        //setting value
        config.setValue("Extent", 20.0);

        //write changes to file
        config.save(); //file now contains "Extent 20.0"

        config.setValue("Extent", 30.0);
    } //write changes to file
     //file now contains "Extent 30.0"
\endcode

*/


class Config
{
public:
    //!Default constructor
    Config();
    //! Destroys the Config object and frees its resources. Saves any changes had made.
    ~Config();
    //!Constructs a new Config object that reads the config file 'path'.
    Config(const char *path);
    //!Returns the value associated with the settings name (key). If there is no setting with the specified name, Config returns defaultValue.
    double getValue(const char* key, double defaultValue);
    //!Stores the value associated with the key. If there already exists a setting with the same key, the existing value is overwritten by the new value.
    double setValue(const char* key, double value);
    //!Save all cached changes to the file.
    void save();
 private:
    QSettings* _settings;
    bool _dirty;
    void bind_file(const QString &path);
};

#endif // CONFIG_H
