#pragma once

#include <string>
#include <Borealis.hpp>
#include "Explorer.hpp"
#include <filesystem>
namespace fs = std::filesystem;

class PlaylistMakerApp {
public:
    static TabFrame *rootFrame;
    static ListItem *crashItem;
    static fs::path path; 
    static Explorer *explorer;
    static std::string rmsgpack_dom_value_print(struct rmsgpack_dom_value *obj);
    static std::string pingDB(const char* path, const char* query_exp);
    PlaylistMakerApp();
    ~PlaylistMakerApp();
    
};