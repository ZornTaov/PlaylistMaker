
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
string PlaylistMakerApp::path;
ListItem *PlaylistMakerApp::pathItem;
using namespace std;

void PlaylistMakerApp::generateFolderView(string path, List* folderContents)
{
    debug(path.c_str());
    DIR* dir;
    struct dirent* ent;
    dir = opendir((path + "/").c_str());//Open current-working-directory.
    if(dir==NULL)
    {
        error("Failed to open dir.");
    }
    else
    {
        //LOG(DEBUG)<<"Dir-listing for "<< path;
        PlaylistMakerApp::path = path;
        while ((ent = readdir(dir)))
        {
            if (string(ent->d_name) == ".")
            {
                continue;
            }
            
            ListItem *fileLabel = new ListItem(ent->d_name);
            debug("%s", fileLabel->getLabel().c_str());
            fileLabel->setClickListener([](View *view)
            {
                debug("clicked %s", ((ListItem*)view)->getLabel().c_str());
                string newPath;
                if(((ListItem*)view)->getLabel() == "..")
                {
                    debug("found ..");
                    newPath = PlaylistMakerApp::path.substr(0,PlaylistMakerApp::path.find_last_of('/'));
                }
                else if(((ListItem*)view)->getLabel() != ".")
                {
                    debug("found dir or file");
                    newPath = PlaylistMakerApp::path + "/" + ((ListItem*)view)->getLabel();
                }
                if(((ListItem*)view)->getLabel().length() - ((ListItem*)view)->getLabel().find_last_of(".") != 4)
                {
                    debug("clicked folder");
                    //should be a file?
                    while(((List*)(view->getParent()))->getViewsCount() > 1)
                    {
                        ((List*)(view->getParent()))->removeView(1, true);
                        ((List*)(view->getParent()))->invalidate();
                    }
                    ((List*)(view->getParent()))->setFocusedIndex(0);
                    Application::requestFocus(((List*)(view->getParent())), FocusDirection::NONE);
                    PlaylistMakerApp::pathItem->setValue(newPath);
                    generateFolderView(newPath,  (List*)(view->getParent()));

                }
                
                
            });
            folderContents->addView(fileLabel);

        }

        closedir(dir);
        //LOG(DEBUG)<<("Done.");
    }
}

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
    rootFrame->setTitle("Borealis Example App");

    List *testList = new List();
    ListItem *themeItem = new ListItem("TV Resolution");
    themeItem->setValue("Automatic");
    themeItem->setClickListener([](View *view) {
#ifdef __SWITCH__
        string path = "/retroarch/database/rdb/Nintendo - Super Nintendo Entertainment System.rdb";
#else
        string path = "libretro-database/rdb/Nintendo - Super Nintendo Entertainment System.rdb";
#endif
        stringstream ss;
        ss << "{'serial':b'534E532D534F2D555341'}";
        printLine("debug", ss.str());
        string str = PlaylistMakerApp::pingDB(path.c_str(),ss.str().c_str());//"{'name':glob('Soul Blazer*')}");
        printLine("DB",str);
        stringstream ss2;
        ss2 << "{'sha1':b'F2832EB02547C39CAE3BDAAB5C2A53E4F8B31810'}";
        printLine("debug", ss2.str());
        string str2 = PlaylistMakerApp::pingDB(path.c_str(),ss2.str().c_str());//"{'name':glob('Soul Blazer*')}");
        printLine("DB",str2);
        stringstream ss3;
        ss3 << "{'md5':b'83CF41D53A1B94AEEA1A645037A24004'}";
        printLine("debug", ss3.str());
        string str3 = PlaylistMakerApp::pingDB(path.c_str(),ss3.str().c_str());//"{'name':glob('Soul Blazer*')}");
        printLine("DB",str3);
#ifdef __MINGW32__
        fflush(0);
#endif
    });
    ListItem *jankItem = new ListItem("User Interface Jank", "Some people believe homebrews to have a jank user interface. Set to Minimal to have a native look and feel, set to Maximal to have a SX OS look and feel.");
    jankItem->setValue("Minimal");

    PlaylistMakerApp::crashItem = new ListItem("Divide by 0", "Can the Switch do it?");
    crashItem->setClickListener([](View *view){ Application::crash("The software was closed because an error occured:\nSIGABRT (signal 6)"); });
    
    ListItem *installerItem = new ListItem("Open example installer");
    jankItem->setClickListener([](View *view){
        if (PlaylistMakerApp::crashItem != nullptr)
        {
            PlaylistMakerApp::crashItem->collapse();
        }
            
    });
    

    testList->addView(themeItem);
    testList->addView(jankItem);
    testList->addView(crashItem);
    testList->addView(installerItem);

    Label *testLabel = new Label(LabelStyle::REGULAR, "For more information about how to use Nintendo Switch and its features, please refer to the Nintendo Support Website on your smart device or PC.", true);
    testList->addView(testLabel);

    rootFrame->addTab("First tab", testList);
    //rootFrame->addTab("Second tab", new Rectangle(nvgRGB(0, 0, 255)));
    rootFrame->addSeparator();
    //rootFrame->addTab("Third tab", new Rectangle(nvgRGB(255, 0, 0)));
    //rootFrame->addTab("Fourth tab", (Rectangle*)(new Rectangle(nvgRGB(0, 255, 0))));
    List* settingsTab = new List();
    ListItem* tglRomPath = new ListItem("Toggle Rom Path","Cycle through known rom paths.");
    tglRomPath->setValue(PMSettings::getRomPath());
    tglRomPath->setClickListener([](View *view)
    { 
        ((ListItem*)view)->setValue(PMSettings::getNextRomPath());
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
    //List *folderTab = new List();
    List *folderContents = new List();
    PlaylistMakerApp::path = PMSettings::getRomPath();
    PlaylistMakerApp::pathItem = new ListItem(path);
    folderContents->addView(PlaylistMakerApp::pathItem);
    generateFolderView(path, folderContents);
    //folderTab->addView(folderContents);
    
    rootFrame->addTab("folder tab", folderContents);

}
