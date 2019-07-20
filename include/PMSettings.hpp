

#include "PMjson.hpp"
#pragma once
#include <string>

using std::string;
class PMSettings
{
public:
    static void Startup();
    static void updateSettings();
    static std::string getCoreFolder(PMjson core);
    static std::string getRomPath();
    static bool isZip(std::string ext);
    static bool useAllExt(std::string ext, PMjson core);
    static bool useSystemExt(std::string ext, PMjson core);
    static bool useAllExtFallback(std::string ext, PMjson core);
    static PMjson Settings;
    static PMjson Systems;
    static std::vector<std::string> zipTypes;
};