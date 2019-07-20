#include "PMSettings.hpp"
#include "Playlist.hpp"
#include <Logger.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sys/stat.h>
#ifdef __SWITCH__
#include <switch.h>
#endif
#ifdef _WIN32 
#include <direct.h> 
#endif
#include <typeinfo>
template<class Element, class Container>
bool array_contains(const Element & element, const Container & container)
{
    return std::find(std::begin(container), std::end(container), element)
            != std::end(container);
}

PMjson PlaylistEntry::DEFAULT_PLAYLIST;
PMjson PlaylistEntry::DEFAULT_PLAYLIST_ENTRY;
PMjson PlaylistEntry::DEFAULT_SYSTEM_ENTRY;

PMjson PMSettings::Settings;
PMjson PMSettings::Systems;
std::vector<std::string> PMSettings::zipTypes = {"7z", "zip", "gz"};
void PMSettings::Startup()
{
    PlaylistEntry::DEFAULT_PLAYLIST = PMjson::parse("{\"version\": \"1.0\",\"items\": []}");
    PlaylistEntry::DEFAULT_PLAYLIST_ENTRY = PMjson::parse("{\"name\": \"name\",\"cores\": [\"core\"],\"system\": [\"system\"],\"allExt\": [\"ext\"],\"systemExt\": [\"ext\"]}");

#ifndef __SWITCH__
    std::string ASSET_LOC = "./resources/";
#else
    std::string ASSET_LOC = "romfs:/";
    romfsInit();
#endif
    //load systems
    std::ifstream iSystemsDefault((ASSET_LOC + "systems.json").c_str());
    iSystemsDefault >> PMSettings::Systems;
    iSystemsDefault.close();
    
    //load settings
    std::fstream iSettings("settings.json");
    if(iSettings.good())
    {
        iSettings >> PMSettings::Settings;
        iSettings.close();
    }
    else
    {
        debug("Unable to load settings.json, getting default.");
        iSettings.close();
        std::ifstream iSettingsDefault((ASSET_LOC + "settings.json").c_str());
        iSettingsDefault >> PMSettings::Settings;
        iSettingsDefault.close();
        PMSettings::updateSettings();
    }
#ifdef __SWITCH__
    romfsExit();
#endif
}
void PMSettings::updateSettings()
{
    std::ofstream iSettings("settings.json");
    iSettings << std::setw(4) << PMSettings::Settings << std::endl;
    iSettings.close();
}
std::string PMSettings::getCoreFolder(PMjson core)
{
    return PMSettings::Settings["useShorthandName"] ? core["name"] : core["system"][0];
}
std::string PMSettings::getRomPath()
{
    struct stat st = {0};
    std::string path = PMSettings::Settings["romsPaths"][PMSettings::Settings["indexRomPathUsed"].get<int>()];
    if (stat(path.c_str(), &st) == -1)
    {
#if defined(_WIN32)
        _mkdir(path.c_str());
#else 
        mkdir(path.c_str(), 0700);
#endif
    }
    return path;
    
}
bool PMSettings::isZip(std::string ext)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return array_contains(ext, PMSettings::zipTypes);
}

bool PMSettings::useAllExt(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return PMSettings::Settings["useAllExtentions"] && array_contains(ext, core["allExt"]);
}

bool PMSettings::useSystemExt(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return core["systemExt"].size() != 0 && array_contains(ext, core["systemExt"]);
}

bool PMSettings::useAllExtFallback(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return !PMSettings::Settings["useAllExtentions"] && core["systemExt"].size() == 0 && 
                        array_contains(ext, core["allExt"]);
}