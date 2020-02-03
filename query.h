
#ifndef QUERY_H
#define QUERY_H


//3rd party includes
#include <Windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <iostream>
#include <ios>
#include <string>
#include <vector>

#pragma comment(lib, "Pdh.lib")

#define PDH_MAX_COUNTER_PATH 2048
static std::string CounterPath(std::string object_name, std::string counter_name, std::string instance_name) {
    PDH_COUNTER_PATH_ELEMENTS_A path_elements = { 0 };
    path_elements.szObjectName = &object_name[0];
    path_elements.szCounterName = &counter_name[0];
    path_elements.szInstanceName = &instance_name[0];
    std::string path(PDH_MAX_COUNTER_PATH + 1, '\0');
    DWORD len = PDH_MAX_COUNTER_PATH;
    auto status = PdhMakeCounterPathA(&path_elements, &path[0], &len, 0);
    if (status != ERROR_SUCCESS)
    {
     //   std::cout << std::hex << status << '\n';
        return std::string("Error");
    }
    path.resize(len - (path[len - 1] == '\0'));
    return path;
}

using namelist_t = std::vector<std::string>;

// no arguments overload gives an empty name list
static namelist_t NameListParser() { return namelist_t(); }

static namelist_t NameListParser(const std::string& buffer)
{
    namelist_t names;
    auto iter = buffer.begin();
    do {
        std::string name;
        while (iter != buffer.end() && *iter) {
            name += *(iter++);
        }
        if (!name.empty()) {
            names.push_back(name);
        }
    } while (iter != buffer.end() && ++iter != buffer.end() && *iter);
    return names;
}

template <typename T>
static void DumpList(const T& list, const char* item_separator = nullptr, const char* item_prefix = nullptr, const char* list_prefix = nullptr, const char* list_postfix = nullptr) {
    if (!item_separator) item_separator = "\n";
    if (!item_prefix) item_prefix = "    ";
    if (!list_prefix) list_prefix = "";
    if (!list_postfix) list_postfix = "\n";

   // std::cout << list_prefix;
    bool first = true;
    for (const auto& elem : list)
    {
        if (first)
        {
          first = false;
        }
        else
        {
          std::cout << item_separator;
        }
        std::cout << item_prefix << elem;
    }
   // std::cout << list_postfix;
}

static namelist_t ListObjectNames() {
    DWORD buflen = 0;

    const DWORD detail_level = PERF_DETAIL_WIZARD;
    PdhEnumObjectsA(0, 0, 0, &buflen, detail_level, TRUE);

    std::string namebuf(buflen, '\0');
    auto status = PdhEnumObjectsA(0, 0, &namebuf[0], &buflen, detail_level, FALSE);

    if (status != ERROR_SUCCESS)
    {
        return NameListParser();
    }
    return NameListParser(namebuf);
}

struct CounterNames { namelist_t counters, instances; };

static CounterNames ListCounters(const std::string& object_name) {
    DWORD counter_list_size = 0;
    DWORD instance_list_size = 0;
    const DWORD detail_level = PERF_DETAIL_WIZARD;
    PdhEnumObjectItemsA(0, 0, object_name.c_str(), 0, &counter_list_size, 0, &instance_list_size, detail_level, 0);
    std::string counter_buf(counter_list_size, '\0');
    std::string inst_buf(instance_list_size, '\0');
    auto status = PdhEnumObjectItemsA(0, 0, object_name.c_str(), &counter_buf[0], &counter_list_size, &inst_buf[0], &instance_list_size, detail_level, 0);
    if (status != ERROR_SUCCESS)
    {
        return CounterNames();
    }

    auto counters = NameListParser(counter_buf);
    auto instances = NameListParser(inst_buf);


    return{ counters, instances };
}

class Query {
    struct CounterData {
        PDH_HCOUNTER hcounter;
        std::string  name;
        std::string  path;
    };

    PDH_HQUERY               hquery;
    std::vector<CounterData> counter_list;
    std::string              object_name;
    std::string              instance_name;

    volatile bool keep_going;

public:
    Query() {
        PdhOpenQuery(0, 0, &hquery) == ERROR_SUCCESS || (hquery = 0);
    }
    Query(const Query&) = delete;
    Query(Query&& src) : hquery(src.hquery) {
        src.hquery = 0;
        // std::move these if you want
        // this is really just for keeping hquery unique
        counter_list = src.counter_list;
        object_name = src.object_name;
        instance_name = src.instance_name;
    }
    Query& operator = (const Query&) = delete;
    Query& operator = (Query&& src) {
        if (hquery) PdhCloseQuery(hquery);
        hquery = src.hquery;
        src.hquery = 0;
        counter_list = src.counter_list;
        object_name = src.object_name;
        instance_name = src.instance_name;
        return *this;
    }
    ~Query() {
        if (!hquery) return;
        PdhCloseQuery(hquery);
        // Counter handles belong to the query and
        // don't appear to need separate closing by
        // the user.
    }
    operator PDH_HQUERY () const { return hquery; }
    bool IsOk() const { return !!hquery; }
    operator const void* () const { return (const void*)IsOk(); }

    void SetInstance(const std::string& name) { instance_name = name; }

    std::vector<std::string> SetObject(const std::string& name,const int instanceIndex) {
        object_name = name;
        auto counter_names = ListCounters(name);
        std::vector<std::string> vect;
        if (counter_names.instances.size())
        {
        //    std::cout << "Automatically selecting instance \"" << counter_names.instances[0] << "\"\n";
            SetInstance(counter_names.instances[instanceIndex]);
        }

        for(int i =0;i<counter_names.instances.size();i++)
        {
          vect.push_back(counter_names.instances[i]);
        }
        return vect;


    }

    static void DumpAvailableObjects()
    {
        DumpList(ListObjectNames());
    }

    void DumpAvailableCounters() const
    {
        if (object_name.empty())
        {
            std::cout << "DumpAvailableCounters: An object needs to be set before calling\n";
            return;
        }
        auto counter_names = ListCounters(object_name);
        DumpList(counter_names.counters);
    }
    void DumpAvailableInstances() const {
        if (object_name.empty())
        {
            std::cout << "DumpAvailableInstances: An object needs to be set before calling\n";
            return;
        }
        auto counter_names = ListCounters(object_name);
        DumpList(counter_names.instances);
    }

    void AddCounter() {}

    template <typename ...more>
    void AddCounter(const std::string& name, more... args) {
        if (!hquery)
        {
            std::cout << "AddCounter: Query was not successfully created\n";
            return;
        }
        if (object_name.empty())
        {
            std::cout << "AddCounter: No Object Name selected\n";
            return;
        }
        if (instance_name.empty())
        {
          //  std::cout << "AddCounter: No Instance Name selected\n";
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

    double CounterPollingDump() {
        if (counter_list.empty())
        {
           // std::cout << "CounterPollingDump: Nothing to do, the Counter List is empty\n";
            return 0;
        }
        size_t max_name_len = 0;
        for (const auto& counter : counter_list)
        {
            if (counter.name.length() > max_name_len) max_name_len = counter.name.length();
        }

            auto status = PdhCollectQueryData(hquery);
            if (status != ERROR_SUCCESS)
            {
               // std::cout << "CounterPollingDump: PdhCollectQueryData failed: " << std::hex << status << '\n';
                return 0;
            }
         //   std::cout << "      =======================\n";
            double counterValue;

                DWORD counter_type;
                PDH_FMT_COUNTERVALUE fmt_value;
                auto status1 = PdhGetFormattedCounterValue(counter_list[0].hcounter, PDH_FMT_DOUBLE, &counter_type, &fmt_value);
                if (status1 != ERROR_SUCCESS)
                {
                    if (status1 == PDH_INVALID_DATA)
                    {
                        std::cout << " -- no data --\n";

                    }
                  //  std::cout << "CounterPollingDump: PdhGetFormattedCounterValue failed: " << std::hex << status1 << '\n';
                    return 0;
                }
               // counterValue=fmt_value.doubleValue;
               std::cout << "    " << fmt_value.doubleValue << '\n';

            return counterValue;
    }


};

#endif // QUERY_H
