/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/




#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <Borealis.hpp>
//#include "easylogging++.h"

#include "PMjson.hpp"
#include "Playlist.hpp"
#include "PMSettings.hpp"
// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <sys/stat.h>
#include <libretrodb.h>
#include <dirent.h>
#ifdef _WIN32 
#include <direct.h> 
#endif
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


using namespace std;
//static int counter = 0;
class PlaylistMakerApp {
public:
    TabFrame *rootFrame;
    static ListItem *crashItem;
    static std::string path;
    static void generateFolderView(string path, List* folderContents)
    {
        debug(path.c_str());
        DIR* dir;
        struct dirent* ent;
        dir = opendir(path.c_str());//Open current-working-directory.
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
                if (std::string(ent->d_name) == ".")
                {
                    continue;
                }
                
                ListItem *fileLabel = new ListItem(ent->d_name);
                debug("%s", fileLabel->getLabel().c_str());
                fileLabel->setClickListener([](View *view)
                {
                    std::string newPath;
                    if(((ListItem*)view)->getLabel() == "..")
                    {
                        newPath = PlaylistMakerApp::path.substr(0,PlaylistMakerApp::path.find_last_of('/'));
                        debug("found ..");
                    }
                    else if(((ListItem*)view)->getLabel() != ".")
                    {
                        newPath = PlaylistMakerApp::path + "/" + ((ListItem*)view)->getLabel();
                    }
                    if(((ListItem*)view)->getLabel().length() - ((ListItem*)view)->getLabel().find_last_of(".") != 4)
                    {
                        //should be a file?
                        while(((List*)(view->getParent()))->getViewsCount() > 0)
                        {
                            ((List*)(view->getParent()))->removeView(0, true);
                            ((List*)(view->getParent()))->invalidate();
                        }
                        ListItem *pathItem = new ListItem(newPath);
                        ((List*)(view->getParent()))->addView(pathItem);
                        ((List*)(view->getParent()))->setFocusedIndex(0);
                        Application::requestFocus(((List*)(view->getParent())), FocusDirection::NONE);
                        generateFolderView(newPath,  (List*)(view->getParent()));

                    }
                    
                    
                });
                folderContents->addView(fileLabel);

            }

            closedir(dir);
            //LOG(DEBUG)<<("Done.");
        }
    }

    static std::string rmsgpack_dom_value_print(struct rmsgpack_dom_value *obj)
    {
        unsigned i;
        std::stringstream ss;
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
                ss << std::dec << (int64_t)obj->val.int_;
                break;
            case RDT_UINT:
                ss << std::dec << (uint64_t)obj->val.uint_;
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
    static std::string pingDB(const char* path, const char* query_exp)
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
    PlaylistMakerApp() {

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
            std::stringstream ss;
            ss << "{'serial':b'534E532D534F2D555341'}";
            printLine("debug", ss.str());
            std::string str = PlaylistMakerApp::pingDB(path.c_str(),ss.str().c_str());//"{'name':glob('Soul Blazer*')}");
            printLine("DB",str);
            std::stringstream ss2;
            ss2 << "{'sha1':b'F2832EB02547C39CAE3BDAAB5C2A53E4F8B31810'}";
            printLine("debug", ss2.str());
            std::string str2 = PlaylistMakerApp::pingDB(path.c_str(),ss2.str().c_str());//"{'name':glob('Soul Blazer*')}");
            printLine("DB",str2);
            std::stringstream ss3;
            ss3 << "{'md5':b'83CF41D53A1B94AEEA1A645037A24004'}";
            printLine("debug", ss3.str());
            std::string str3 = PlaylistMakerApp::pingDB(path.c_str(),ss3.str().c_str());//"{'name':glob('Soul Blazer*')}");
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
        rootFrame->addTab("Second tab", new Rectangle(nvgRGB(0, 0, 255)));
        rootFrame->addSeparator();
        rootFrame->addTab("Third tab", new Rectangle(nvgRGB(255, 0, 0)));
        rootFrame->addTab("Fourth tab", (Rectangle*)(new Rectangle(nvgRGB(0, 255, 0))));

        //List *folderTab = new List();
        List *folderContents = new List();
        PlaylistMakerApp::path = PMSettings::getRomPath();
        ListItem *pathItem = new ListItem(path);
        folderContents->addView(pathItem);
        generateFolderView(path, folderContents);
        //folderTab->addView(folderContents);
        
        rootFrame->addTab("folder tab", folderContents);

    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        // Init the app
        setLogLevel(LogLevel::DEBUG);

        if (!Application::init(StyleEnum::ACCURATE))
        {
            error("Unable to init Borealis application");
            return EXIT_FAILURE;
        }
        //validatePath("logs");
        //assuming on unix or switch
        struct stat st = {0};
        if (stat("logs", &st) == -1)
        {
#if defined(_WIN32)
                _mkdir("logs");
#else 
                mkdir("logs", 0700);
#endif
        }
        //setup logs, one per day
        /*el::Configurations c;
//        c.setGlobally(el::ConfigurationType::Format, "%datetime{%a %b %d, %H:%m} %msg");
        c.setGlobally(el::ConfigurationType::Filename, "logs/PlaylistMaker_%datetime{%Y%M%d}.log");
        el::Loggers::setDefaultConfigurations(c, true);*/
	    PMSettings::Startup();
        
        
        /* scoped variables */ {
            PlaylistMakerApp* app = new PlaylistMakerApp();

        // Add the root view to the stack
        Application::pushView(app->rootFrame);
            // Run the app
            while (Application::mainLoop());

            // Exit
            return EXIT_SUCCESS;
        }
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << endl;
        error(error_msg.c_str());
        throw e;
        //return -1;
    }

    return 0;
}
ListItem *PlaylistMakerApp::crashItem;
std::string PlaylistMakerApp::path;