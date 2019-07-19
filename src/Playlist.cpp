#include "Playlist.hpp"
//#include "easylogging++.h"
#include <Logger.hpp>
#include "CRC.h"
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <locale>  
#include <fstream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <dirent.h>
#include <algorithm>
#include <string>
#ifdef __SWITCH__
#include <switch.h>
#endif
#ifdef _WIN32 
#include <direct.h> 
#endif
PMjson PlaylistEntry::DEFAULT_PLAYLIST;
PMjson PlaylistEntry::DEFAULT_PLAYLIST_ENTRY;
PMjson PlaylistEntry::DEFAULT_SYSTEM_ENTRY;

PMjson PMSettings::Settings;
PMjson PMSettings::Systems;
std::vector<std::string> PMSettings::zipTypes = {"7z", "zip", "gz"};

template<class T, size_t N>
constexpr size_t size(T (&)[N]) { return N; }

void PlaylistEntry::Startup()
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
void PlaylistEntry::PrintPlaylistEntry(PMjson entry)
{
    info("%s", entry.dump(2));//["name"].get<std::string>();
}
PMjson PlaylistEntry::generatePlaylistEntry(std::string fileName, std::string ext, std::string romDir, std::string systemName, std::string core)
{
    bool useCRC32 = true;
    std::string core_path;
    std::string romcrc;
    if(core.compare(""))
    {
        core_path = PMSettings::Settings["romsPaths"].get<std::string>()+core+"_libretro_libnx.nro";
    }
    else
    {
        core_path = core = "DETECT";
    }
    if (useCRC32)
    {
        romcrc = PlaylistEntry::getCRC(romDir+"/"+fileName+"."+ext);
    }
    else
    {
        romcrc = "DETECT";
    }
    debug("Making json entry for ", romDir+"/"+fileName+"."+ext);

    PMjson info = {
        {"path", romDir+"/"+fileName+"."+ext},
        {"label", fileName},
        {"core_path", core_path},
        {"core_name", core},
        {"crc32", romcrc},
        {"db_name", systemName+".lpl"}
    };
    return info;
}
PMjson PlaylistEntry::oldToNew(std::string entry[])
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
std::string PlaylistEntry::getCRC(std::string path)
{
    
    try
    {
        std::ifstream romfile;
        std::uint32_t crc = 0;
         char *buffer = NULL;
        std::streamsize const  buffer_size = 1048576;
        romfile.open(path, std::ios_base::binary);
        //LOG(DEBUG)<<"Getting CRC32 on " << path;
        if(romfile)
        {
            buffer = ( char*)malloc(buffer_size);
            unsigned i = 0;
            do
            {
                romfile.read(buffer, buffer_size);
                //LOG(DEBUG) << romfile.gcount(); 
                if (romfile.eof() || romfile.gcount() == 0)
                {
                    break;
                }
                crc = CRC::Calculate(buffer, romfile.gcount(), CRC::CRC_32(), crc);
                //LOG(DEBUG) << std::hex << crc;
                i++;
            }
            while ( i < 64 );
            free(buffer);
            romfile.close();
            std::stringstream os;
            os << std::hex << std::uppercase << crc;
            return os.str() + "|crc";
        }
        else romfile.close();
        
    }
    catch(const std::exception& e)
    {
        std::cerr << "CRC32 " << e.what() << '\n';
        //romfile.close();
    }
    return "DETECT";
}


#include <typeinfo>
template<class Element, class Container>
bool array_contains(const Element & element, const Container & container)
{
    return std::find(std::begin(container), std::end(container), element)
            != std::end(container);
}
void rtrim(std::string& s, const std::string& delimiters = " \f\n\r\t\v" )
{
   s.erase( s.find_last_not_of( delimiters ) + 1 );
}
std::vector<std::string> split (const string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}
bool isdir(std::string path)
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
std::string getCoreFolder(PMjson core)
{
    return PMSettings::Settings["useShorthandName"] ? core["name"] : core["system"][0];
}
void Playlist::validateFolders()
{
    for(PMjson core : PMSettings::Systems)
    {
        struct stat st = {0};
        info("validating rom folder at %s", PMSettings::getRomPath()+getCoreFolder(core));
        std::string path = PMSettings::getRomPath()+getCoreFolder(core);
        if(stat(path.c_str(), &st) == -1)
        {
            info("Missing %s", PMSettings::getRomPath()+getCoreFolder(core));
#if defined(_WIN32)
            _mkdir(path.c_str());
#else 
            mkdir(path.c_str(), 0700);
#endif
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
bool isZip(std::string ext)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return array_contains(ext, PMSettings::zipTypes);
}

bool useAllExt(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return PMSettings::Settings["useAllExtentions"] && array_contains(ext, core["allExt"]);
}

bool useSystemExt(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return core["systemExt"].size() != 0 && array_contains(ext, core["systemExt"]);
}

bool useAllExtFallback(std::string ext, PMjson core)
{
    std::locale loc;
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return !PMSettings::Settings["useAllExtentions"] && core["systemExt"].size() == 0 && 
                        array_contains(ext, core["allExt"]);
}
/* void addToPlaylist(system, entries, asJson=False)
{
    LOG(DEBUG) << "Starting addToPlaylist";
    if( !isdir(Settings["playlistsPath"]+system+".lpl") )
    {
        LOG(DEBUG) << "Found playlist " + system;
        //there's already a file, so read from it to get what's already there
        std::vector<PMjson> newentries;
        std::vector<PMjson> known;
        PMjson jsonPlaylist;
        char lastChar;
        char firstChar;
        std::fstream playlist(Settings["playlistsPath"]+system+".lpl", "r", newline='');
        {
            firstChar = playlist.read(1);
            
            if( firstChar != '\0' )
            {
                playlist.seek(0, os.SEEK_SET);
                if( firstChar in ["{","["] )
                {
                    LOG(DEBUG) << "playlist is json";
                    //convert from old to new format
                    //std::vector<PMjson> entries = [toOrderedDict(oldToNew(entries[i:i+6]), keyOrder) for( i in range(0,len(entries),6)] );

                    //jsonPlaylist = json.load(playlist, object_pairs_hook=OrderedDict);
                    //get list of known entries
                    //known = [x["path"] for( x in jsonPlaylist["items"]] );
                    //check iif entry is known
                    for( PMjson entry : entries )
                    {
                        if( entry["path"] not in known )
                        {                          
                            newentries.append(entry);
                        }
                    }
                }
                else
                {
                    LOG(DEBUG) << "playlist is old format";
                    //get list of known entries
                    line = playlist.readline();
                    while( line )
                    {
                        known.append(line[:-1]);//os.path.split(line)[1][:-1])
                        playlist.readline();//playlist are always multiple of 6
                        playlist.readline();
                        playlist.readline();
                        playlist.readline();
                        lastLine = playlist.readline();
                        lastChar = lastLine[:-1];
                        line = playlist.readline();
                        if( lastChar == "\n" )
                        {
                            LOG(INFO) << "\\n";
                        }
                        if( line == "" or lastChar == "\n" )
                        {
                            lastChar = "\n";
                            break;
                        }
                    }
                    //check if( entry is known )
                    for( int i = 0; i < entries.size(); i+=6)//i in range(0,len(entries),6) )
                    {
                        if( entries[i] not in known: //os.path.split(entries[i])[1] )
                        {
                            newentries.extend(entries[i:i+6]);
                        }
                    }
                }
            }
        }
        if( firstChar in ["{","["] )
        {
            //add newentries to existing playlist
            if( newentries )
            {
                LOG(DEBUG) << "Adding "+str(len(newentries))+" new entries to "+system;
                //write to the file those that are new
                with open(Settings["playlistsPath"]+system+".lpl", "w", newline="\n") as playlist
                {
                    jsonPlaylist["items"].extend(newentries);
                    json.dump(OrderedDict([(key, jsonPlaylist[key]) for( key in templateOrder]), playlist, sort_keys=False, indent=2) );
                }
                LOG(INFO) << system+" playlist modified.";
            }
        }
        else
        {
            //add newentries to existing playlist
            if( newentries )
            {
                LOG(DEBUG) << "Adding "+str(len(newentries))+" new entries to "+system;
                with open(Settings["playlistsPath"]+system+".lpl", "a", newline='') as playlist
                {
                    if( lastChar != "\n" )
                    {
                        playlist.write("\n");
                    }
                    for( entry in newentries )
                    {
                        playlist.write(entry+"\n");
                    }
                LOG(INFO) << system+" playlist modified.";
                }
            }
        }
    }
    else
    {
        LOG(DEBUG) << "Making playlist "+system;
        //first time creation
        with open(Settings["playlistsPath"]+system+".lpl", "w", newline="\n") as playlist
        {
            if( asJson )
            {
                //make json playlist
                plist = jsonTemplate;
                entries = [toOrderedDict(oldToNew(entries[i:i+6]), keyOrder) for( i in range(0,len(entries),6)] );

                plist["items"] = entries;
                json.dump(OrderedDict([(key, plist[key]) for( key in templateOrder]), playlist, sort_keys=False, indent=2) );
            }
            else
            {
                //make old playlist
                for( entry in entries )
                {
                    playlist.write(entry+"\n");
                }
            }
            LOG(INFO) << system+" playlist created";
        }
    }
} */
void Playlist::generatePlaylist()
{
    std::locale loc;
    info("Starting generatePlaylist at ");//+str(datetime.datetime.now()));
    for(PMjson core : PMSettings::Systems)
    {
        if (core["allExt"].size() != 0)
        {
            info("Preparing file list for %s", core["name"]);
            PMjson playlist;
            std::vector<std::string> romList;
            if( isdir(PMSettings::getRomPath()+getCoreFolder(core)))
            {
                DIR* dir;
                struct dirent* ent;
                std::string path = PMSettings::getRomPath()+getCoreFolder(core);
                dir = opendir(path.c_str());//Open current-working-directory.
                if(dir==NULL)
                {
                    error("Failed to open dir.");
                }
                else
                {
                    //LOG(DEBUG)<<"Dir-listing for "<< path;
                    while ((ent = readdir(dir))!= NULL)
                    {
#ifdef __SWITCH__
                        if ( ent->d_type == 0x8)
                            romList.push_back(ent->d_name);
#else
                        debug(ent->d_name[ent->d_namlen-3]+"");
                        if (strcmp(ent->d_name[ent->d_namlen-3]+"",".")==0)
                        {
                            romList.push_back(ent->d_name);
                        }
                        
#endif
                    }

                    closedir(dir);
                    //LOG(DEBUG)<<("Done.");
                }
            }
            //generate entry per rom
            if( romList.size() != 0)
            {
                //LOG(DEBUG) << "found Roms for " << core["name"];
                //filter if there are .m3u's
                if( getCoreFolder(core) == "psx")
                {
                    info("Special Casing psx m3u's");
                    std::vector<std::string> result;
                    for(std::string rom : romList)
                    {
                        std::vector<std::string> romsplit = split(rom, '.');
                        std::transform(romsplit[1].begin(), romsplit[1].end(), romsplit[1].begin(), ::tolower);
                        if( romsplit[1].compare("m3u") == 0)
                        {
                            //load m3u, filter out cue names
                            std::ifstream m3uFile(PMSettings::getRomPath()+getCoreFolder(core)+"/"+rom);
                            
	                        std::string line;
                            while(std::getline(m3uFile, line))
                            {
                                if(line.size() > 0 && line != "\n")
                                {
                                    rtrim(line);
                                    result.push_back(line);
                                }
                            }
                        }
                        
                    }
                    //LOG(DEBUG)<<("ignoring PSX cue's");
                    //LOG(DEBUG)<<(result);
                    romList.erase( remove_if( begin(romList),end(romList),
                        [&](std::string x){return find(begin(result),end(result),x)!=end(result);}), end(romList) );
                }
                for( std::string rom : romList)
                {
                    std::vector<std::string> romsplit = split(rom, '.');
                    //it's a zip file 
                    if (isZip(romsplit[1]) ||
                        //using all extentions
                        useAllExt(romsplit[1], core) || 
                        //use only extentions for system
                        useSystemExt(romsplit[1], core) || 
                        //fallback if systemExt is empty
                        useAllExtFallback(romsplit[1], core))
                    {
                        debug("Adding file %s", rom);
                        std::string coreUsed = core["cores"].size() == 1?core["cores"][0]:"";
                        PMjson romInfo = PlaylistEntry::generatePlaylistEntry(romsplit[0],
                                                               romsplit[1],
                                                               PMSettings::getRomPath()+getCoreFolder(core),
                                                               core["system"][0],
                                                               coreUsed);
                        playlist.push_back(romInfo);
                       info("%s",romInfo);
                    }
                    /* else
                    {
                        LOG(DEBUG) << "Skipped " << rom << " because "<<
                        "isZip: " << isZip(romsplit[1]) << 
                        //using all extentions
                        "useAllExt: " << useAllExt(romsplit[1], core) << 
                        //use only extentions for system
                        "useSystemExt: " << useSystemExt(romsplit[1], core) << 
                        //fallback if systemExt is empty
                        "useAllExtFallback: " << useAllExtFallback(romsplit[1], core);
                    } */
                }
            }
            //for( line : playlist)
            //    logger.info(line)
            //create playlist file
            if( playlist.size() != 0)
            {
                debug("Number of Files Found: %d", playlist.size());
                //addToPlaylist(core["system"][0], playlist, Settings["makeJsonPlaylists"]);
            }
        }
    }
    //state = "Playlists: Complete"
}