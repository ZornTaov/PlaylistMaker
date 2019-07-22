

#include "PMjson.hpp"
#pragma once
#include <string>

using std::string;
class PMSettings
{
private:
    static PMjson Settings;
public:
    static void Startup();
    static void Shutdown();
    static void updateSettings();
    static std::string getCoreFolder(PMjson core);
    static std::string getRomPath();
    static std::string getNextRomPath();
    static bool getUseAllExt();
    static void setUseAllExt(bool);
    static bool getUseShorthand();
    static void setUseShorthand(bool);
    static bool isZip(std::string ext);
    static bool useAllExt(std::string ext, PMjson core);
    static bool useSystemExt(std::string ext, PMjson core);
    static bool useAllExtFallback(std::string ext, PMjson core);
    static PMjson Systems;
    static std::vector<std::string> zipTypes;
};