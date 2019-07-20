

#include "PMjson.hpp"
#pragma once
#include <string>

using std::string;

class PlaylistEntry
{
public:
    static PMjson DEFAULT_PLAYLIST;
    static PMjson DEFAULT_PLAYLIST_ENTRY;
    static PMjson DEFAULT_SYSTEM_ENTRY;

    static void PrintPlaylistEntry(PMjson entry);
    static PMjson generatePlaylistEntry(string fileName, string ext, string romDir, string systemName, string core="");
    static PMjson oldToNew(string oldEntry[]);
    static std::string getCRC(std::string entry);
};

class Playlist
{
public:
    static void validateFolders();
    static void generatePlaylist();
};