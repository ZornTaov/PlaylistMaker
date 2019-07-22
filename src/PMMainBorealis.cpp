
#include <Borealis.hpp>
//#include "easylogging++.h"

#include "PMjson.hpp"
#include "Playlist.hpp"
#include "PMSettings.hpp"
#include "PMApp.hpp"
#include <sys/stat.h>
#ifdef _WIN32 
#include <direct.h> 
#endif
//static int counter = 0;


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