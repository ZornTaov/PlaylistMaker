#include "Playlist.hpp"
#include "easylogging++.h"
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <dirent.h>
#include <switch.h>

PMjson PlaylistEntry::DEFAULT_PLAYLIST;
PMjson PlaylistEntry::DEFAULT_PLAYLIST_ENTRY;
PMjson PlaylistEntry::DEFAULT_SYSTEM_ENTRY;

PMjson PMSettings::Settings;
PMjson PMSettings::Systems;
string PMSettings::zipTypes[3] = {".7z", ".zip", ".gz"};

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }

void PlaylistEntry::Startup()
{
    PlaylistEntry::DEFAULT_PLAYLIST = PMjson::parse("{\"version\": \"1.0\",\"items\": []}");
    PlaylistEntry::DEFAULT_PLAYLIST_ENTRY = PMjson::parse("{\"name\": \"name\",\"cores\": [\"core\"],\"system\": [\"system\"],\"allExt\": [\".ext\"],\"systemExt\": [\".ext\"]}");

    //load systems
    std::ifstream iSystemsDefault("romfs:/systems.json");
    iSystemsDefault >> PMSettings::Systems;
    iSystemsDefault.close();

    //load settings
    std::fstream iSettings("settings.json");
    if(iSettings.good())
    {
        LOG(INFO) << "Loading settings.json." << std::endl;
        iSettings >> PMSettings::Settings;
        iSettings.close();
    }
    else
    {
        LOG(INFO) << "Unable to load settings.json, getting default."<< std::endl;
        iSettings.close();
        std::ifstream iSettingsDefault("romfs:/settings.json");
        iSettingsDefault >> PMSettings::Settings;
        iSettingsDefault.close();
        PMSettings::updateSettings();
    }
}
void PMSettings::updateSettings()
{
    std::ofstream iSettings("settings.json");
    iSettings << std::setw(4) << PMSettings::Settings << std::endl;
    iSettings.close();
}
void PlaylistEntry::PrintPlaylistEntry(PMjson entry)
{
    LOG(INFO) << entry.dump(2);//["name"].get<std::string>();
}
PMjson PlaylistEntry::generatePlaylistEntry(string fileName, string ext, string romDir, string systemName, string core)
{
    string core_path;
    if(core.compare(""))
    {
        core_path = PMSettings::Settings["romsPaths"].get<std::string>()+core+"_libretro_libnx.nro";
    }
    else
    {
        core_path = core = "DETECT";
    }
    PMjson info = {
        {"path", romDir+"/"+fileName+ext},
        {"label", fileName},
        {"core_path", core_path},
        {"core_name", core},
        {"crc32", "DETECT"},
        {"db_name", systemName+".lpl"}
    };
    return info;
}
PMjson PlaylistEntry::oldToNew(string entry[])
{
    PMjson info = {
        {"path", entry[0]},
        {"label", entry[1]},
        {"core_path", entry[2]},
        {"core_name", entry[3]},
        {"crc32", entry[4]},
        {"db_name", entry[5]}
    };
    return info;
}

bool isdir(string path)
{
    struct stat s;
    if( stat(path.c_str(),&s) == 0 )
    {
        if( s.st_mode & S_IFDIR )
        {
            //it's a directory
            return true;
        }
        else if( s.st_mode & S_IFREG )
        {
            //it's a file
            return false;
        }
        else
        {
            //something else
            return false;
        }
    }
    else
    {
        //error
        return false;
    }
}
std::string getRomPath()
{
    struct stat st = {0};
    std::string path = PMSettings::Settings["romsPaths"][PMSettings::Settings["indexRomPathUsed"].get<int>()];
    if (stat(path.c_str(), &st) == -1)
    {
        mkdir(path.c_str(), 0700);
    }
    return path;
    
}
std::string getCoreFolder(PMjson core)
{
    return PMSettings::Settings["useShorthandName"] ? core["name"] : core["system"][0];
}
void Playlist::validateFolders()
{
    for(PMjson core : PMSettings::Systems)
    {
        struct stat st = {0};
        LOG(INFO) << "validating rom folder at "+getRomPath()+getCoreFolder(core);
        std::string path = getRomPath()+getCoreFolder(core);
        if(stat(path.c_str(), &st) == -1)
        {
            LOG(INFO) << "Missing "+getRomPath()+getCoreFolder(core);
            mkdir(path.c_str(), 0700);
            std::ofstream myfile;
            myfile.open(path+"/files.txt");
            {
                myfile << "put files in here ending with: ";
                for (std::string i : PMSettings::zipTypes)
                {
                    myfile << i+", ";
                }
                for (std::string i : core[PMSettings::Settings["useAllExtentions"] || core["systemExt"].size() == 0 ? "allExt" : "systemExt"])
                {
                    myfile << i+", ";
                }
            }
            myfile.close();
        }
    }
    //state = "Validate: Complete";
}