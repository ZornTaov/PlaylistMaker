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


#include <Borealis.hpp>

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#include <iostream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <string>

#include "PMjson.hpp"
#include "Playlist.hpp"
// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <sys/stat.h>
#include <libretrodb.h>
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;
static int counter = 0;
class PlaylistMakerApp {
public:
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
                    ss << std::hex << std::setfill('0') << std::uppercase << std::setw(2) <<(unsigned short) obj->val.binary.buff[i];
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
    static std::string pingDB(char* path, char* query_exp)
    {
        int rv;
        libretrodb_t *db;
        libretrodb_cursor_t *cur;
        libretrodb_query_t *q;
        struct rmsgpack_dom_value item;
        const char *error;
        db  = libretrodb_new();
        cur = libretrodb_cursor_new();
        if (!db || !cur)
        {
            return "";
        }
        error = NULL;
        if ((rv = libretrodb_open(path, db)) != 0)
        {
            printf("Could not open db file '%s': %s\n", path, strerror(-rv));
            return "";
        }
        else
        {
            LOG(DEBUG) << "Opened db file " << path;
        }
        
        q = (libretrodb_query_t*)libretrodb_query_compile(db, query_exp, strlen(query_exp), &error);

        if (error)
        {
            printf("%s\n", error);
            //goto error;
            return "";
        }

        if ((rv = libretrodb_cursor_open(db, cur, q)) != 0)
        {
            printf("Could not open cursor: %s\n", strerror(-rv));
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
        TabFrame *rootFrame = new TabFrame();
        rootFrame->setTitle("Borealis Example App");

        List *testList = new List();
        ListItem *themeItem = new ListItem("TV Resolution");
        themeItem->setValue("Automatic");

        ListItem *jankItem = new ListItem("User Interface Jank", "Some people believe homebrews to have a jank user interface. Set to Minimal to have a native look and feel, set to Maximal to have a SX OS look and feel.");
        jankItem->setValue("Minimal");

        ListItem *crashItem = new ListItem("Divide by 0", "Can the Switch do it?");
        crashItem->setClickListener([](View *view){ Application::crash("The software was closed because an error occured:\nSIGABRT (signal 6)"); });

        ListItem *installerItem = new ListItem("Open example installer");
        installerItem->setClickListener([](View *view) {
            std::string str = PlaylistMakerApp::pingDB((char*)"/retroarch/database/rdb/Nintendo - Super Nintendo Entertainment System.rdb",(char*)"{'name':glob('Soul Blazer*')}");
            LOG(INFO) << str;
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
        rootFrame->addTab("Fourth tab", new Rectangle(nvgRGB(0, 255, 0)));

        // Add the root view to the stack
        Application::pushView(rootFrame);
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
            mkdir("logs", 0700);
        }
        //setup logs, one per day
        el::Configurations c;
//        c.setGlobally(el::ConfigurationType::Format, "%datetime{%a %b %d, %H:%m} %msg");
        c.setGlobally(el::ConfigurationType::Filename, "logs/PlaylistMaker_%datetime{%Y%M%d}.log");
        el::Loggers::setDefaultConfigurations(c, true);
	    PlaylistEntry::Startup();
        

        /* scoped variables */ {
            PlaylistMakerApp* app = new PlaylistMakerApp();

            // Run the app
            while (Application::mainLoop());

            // Exit
            return EXIT_SUCCESS;
        }
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << endl;
        printf(error_msg.c_str());
        throw e;
        //return -1;
    }

    return 0;
}
