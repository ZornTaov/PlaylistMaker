#pragma once

#include <string>
#include <Borealis.hpp>

class PlaylistMakerApp {
public:
    static TabFrame *rootFrame;
    static ListItem *crashItem;
    static std::string path;
    
    static void generateFolderView(string path, List* folderContents);
    static std::string rmsgpack_dom_value_print(struct rmsgpack_dom_value *obj);
    static std::string pingDB(const char* path, const char* query_exp);
    PlaylistMakerApp();
    ~PlaylistMakerApp();
    
};