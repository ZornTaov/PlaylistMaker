
#include "PMApp.hpp"
#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
//#include "easylogging++.h"

#include "PMjson.hpp"
#include "Playlist.hpp"
#include "PMSettings.hpp"
#include "Explorer.hpp"
// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <libretrodb.h>
#include <dirent.h>
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

TabFrame *PlaylistMakerApp::rootFrame;
ListItem *PlaylistMakerApp::crashItem;
fs::path PlaylistMakerApp::path;
Explorer *PlaylistMakerApp::explorer;
using namespace std;

string PlaylistMakerApp::rmsgpack_dom_value_print(struct rmsgpack_dom_value *obj)
{
    unsigned i;
    stringstream ss;
    switch (obj->type)
    {
        case RDT_NULL:
            ss << "nil";
            break;
        case RDT_BOOL:
            if (obj->val.bool_)
                ss << "true";
            else
                ss << "false";
            break;
        case RDT_INT:
            ss << dec << (int64_t)obj->val.int_;
            break;
        case RDT_UINT:
            ss << dec << (uint64_t)obj->val.uint_;
            break;
        case RDT_STRING:
            ss << "\"" << obj->val.string.buff << "\"";
            break;
        case RDT_BINARY:
            ss << "\"";
            for (i = 0; i < obj->val.binary.len; i++)
            {
                char buffer[3];
                snprintf(buffer,3,"%02X", (unsigned char) obj->val.binary.buff[i]);
                ss << buffer;
            }
            ss << "\"";
            break;
        case RDT_MAP:
            ss << "{";
            for (i = 0; i < obj->val.map.len; i++)
            {
                ss << rmsgpack_dom_value_print(&obj->val.map.items[i].key);
                ss << ": ";
                ss << rmsgpack_dom_value_print(&obj->val.map.items[i].value);
                if (i < (obj->val.map.len - 1))
                ss << ", ";
            }
            ss << "}";
            break;
        case RDT_ARRAY:
            ss << "[";
            for (i = 0; i < obj->val.array.len; i++)
            {
                ss << rmsgpack_dom_value_print(&obj->val.array.items[i]);
                if (i < (obj->val.array.len - 1))
                ss << ", ";
            }
            ss << "]";
            break;
    }
    return ss.str();
}
string PlaylistMakerApp::pingDB(const char* path, const char* query_exp)
{
    int rv;
    libretrodb_t *db;
    libretrodb_cursor_t *cur;
    libretrodb_query_t *q;
    struct rmsgpack_dom_value item;
    const char *dberror;
    db  = libretrodb_new();
    cur = libretrodb_cursor_new();
    if (!db || !cur)
    {
        return "";
    }
    dberror = NULL;
    if ((rv = libretrodb_open(path, db)) != 0)
    {
        error("Could not open db file '%s': %s\n", path, strerror(-rv));
        return "";
    }
    else
    {
        debug("Opened db file %s", path);
    }
    
    q = (libretrodb_query_t*)libretrodb_query_compile(db, query_exp, strlen(query_exp), &dberror);

    if (dberror)
    {
        error("%s\n", dberror);
        //goto error;
        return "";
    }

    if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
    {
        error("Could not open cursor: %s\n", strerror(-rv));
        //goto error;
        return "";
    }
    string str = "";
    while (libretrodb_cursor_read_item(cur, &item) == 0)
    {
        str += rmsgpack_dom_value_print(&item);
        str += "\n";
        rmsgpack_dom_value_free(&item);
    }
    libretrodb_cursor_close(cur);
    libretrodb_close(db);
    return str;
}
PlaylistMakerApp::PlaylistMakerApp() {

    // Create a sample view
    rootFrame = new TabFrame();
    rootFrame->setTitle("Playlist Maker");
    PlaylistMakerApp::path = PMSettings::getRomPath();
    explorer = new Explorer(PlaylistMakerApp::path.string());
    //PlaylistMakerApp::pathItem = new ListItem(path);
    //folderContents->addView(PlaylistMakerApp::pathItem);
    explorer->generateFolderView(path);
    //folderTab->addView(folderContents);
    
    rootFrame->addTab("folder tab", explorer);

    rootFrame->addSeparator();

    List* settingsTab = new List();
    ListItem* tglRomPath = new ListItem("Toggle Rom Path","Cycle through known rom paths.");
    tglRomPath->setValue(PMSettings::getRomPath());
    tglRomPath->setClickListener([](View *view)
    { 
        fs::path newpath(PMSettings::getNextRomPath());
        ((ListItem*)view)->setValue(newpath.string());
        PlaylistMakerApp::explorer->clear();
        PlaylistMakerApp::explorer->generateFolderView(newpath);
    });
    ToggleListItem* tglUseAllExtentions = new ToggleListItem("Use All Extentions",PMSettings::getUseAllExt(), "If the Generator should include all known extentions for all emulators for a system, or only use extentions for that system.");
    tglUseAllExtentions->setClickListener([](View *view)
    { 
        PMSettings::setUseAllExt(((ToggleListItem*)view)->getToggleState());
    });
    ToggleListItem* tglUseShorthand = new ToggleListItem("Use Shorthand Name", PMSettings::getUseShorthand(), "Shorthand = 'GB'\nFull = 'Nintendo - Game Boy'");
    tglUseShorthand->setClickListener([](View *view)
    { 
        PMSettings::setUseShorthand(((ToggleListItem*)view)->getToggleState());
    });
    settingsTab->addView(tglRomPath);
    settingsTab->addView(tglUseAllExtentions);
    settingsTab->addView(tglUseShorthand);
    
    rootFrame->addTab("Settings", settingsTab);
}
