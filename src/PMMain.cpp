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

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/colorpicker.h>
#include <nanogui/graph.h>
#include <nanogui/navwidget.h>

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#include <iostream>
#include <string>

#include "PMjson.hpp"
#include "Playlist.hpp"
// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <switch.h>
#include <sys/stat.h>
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

class PlaylistMakerApp : public nanogui::Screen {
public:
    PlaylistMakerApp() : nanogui::Screen(Eigen::Vector2i(1280, 720), "NanoGUI Test") {
        using namespace nanogui;
        
        if (glfwJoystickIsGamepad(GLFW_JOYSTICK_1))
        {
            GLFWgamepadstate state;
            printf("Gamepad detected: %s\n", glfwGetGamepadName(GLFW_JOYSTICK_1));
            glfwGetGamepadState(GLFW_JOYSTICK_1, &state);
        }
        setBackground(Color(235, 235, 235, 255));
        auto *windowLayout = new GroupLayout();
        auto *contentLayout = new GroupLayout();
        Window* baseWindow = new Window(this);
        baseWindow->setPosition(Vector2i(0,0));
        baseWindow->setLayout(new GroupLayout());
        
        windowLayout->setMargin(0);
        contentLayout->setMargin(20);
        contentLayout->setSpacing(0);

        NavWidget* window = baseWindow->add<NavWidget>();

        window->setFixedHeight(720);
        window->setFixedWidth(1280);
        window->setHeight(720);
        window->setWidth(1280);
        Widget* layer = window->createNav("Color Wheel");
        layer->setLayout(contentLayout);

        // Use overloaded variadic add to fill the nav widget with Different navs.
        Button *b = layer->add<Button>();
        b->setFlags(Button::ToggleButton);
        b->setChangeCallback([](bool state) { printf("Toggled %d\n", state); });
        
        b = layer->add<Button>();
        b->setCallback([] { printf("pushed!\n"); });
        layer = window->createNav("Function Graph");
        layer->setLayout(new GroupLayout());

        layer->add<Label>("Function graph widget", "sans");

        Graph *graph = layer->add<Graph>("Some Function");

        graph->setHeader("E = 2.35e-3");
        graph->setFooter("Iteration 89");
        VectorXf &func = graph->values();
        func.resize(100);
        for (int i = 0; i < 100; ++i)
            func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
                              0.5f * std::cos(i / 23.f) + 1);

        // A simple counter.

        performLayout();

        /* All NanoGUI widgets are initialized at this point. Now
           create an OpenGL shader to draw the main window contents.

           NanoGUI comes with a simple Eigen-based wrapper around OpenGL 3,
           which eliminates most of the tedious and error-prone shader and
           buffer object management.
        */

    }

    virtual bool gamepadButtonEvent(int jid, int button, int action)
    {
        /* if (Screen::gamepadButtonEvent(jid, button, action))
        {
            printf("handled");
            return true;
        } */
        if (button == GLFW_GAMEPAD_BUTTON_X && action == GLFW_PRESS)
        {
//            PlaylistEntry::PrintPlaylistEntry(PlaylistEntry::DEFAULT_PLAYLIST_ENTRY);
            Playlist::validateFolders();
            //PlaylistEntry::PrintPlaylistEntry(PlaylistEntry::generatePlaylistEntry("name", "ext", "romDir", "systemName", "core"));
        }
        if (button == GLFW_GAMEPAD_BUTTON_START && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(glfwWindow(), GLFW_TRUE);
            return true;
        }
        return false;
    }
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            printf("escape pressed!\n");
            //setVisible(false);
            return true;
        }
        //if (action != 0)
            //printf("b%i, ",key);
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        
        // Gamepad
        /*GLFWgamepadstate gamepad = {};
        if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        {
            // Gamepad not available, so let's fake it with keyboard
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  = glfwGetKey(glfwWindow(), GLFW_KEY_LEFT);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = glfwGetKey(glfwWindow(), GLFW_KEY_RIGHT);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]    = glfwGetKey(glfwWindow(), GLFW_KEY_UP);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  = glfwGetKey(glfwWindow(), GLFW_KEY_DOWN);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_START]      = glfwGetKey(glfwWindow(), GLFW_KEY_ESCAPE);
        }*/
        /*
        int butt = 0;
        for (;butt < 16; butt++)
        //while (butt < 16)
        {
            if(gamepad.buttons[butt] == GLFW_PRESS)
            {
                
            }
        }*/
        // Exit by pressing Start (aka Plus)
        /*if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS)
        {
            printf("start pressed!\n");
            glfwSetWindowShouldClose(glfwWindow(), GLFW_TRUE);
            //return true;
        }*/
        
        /* Draw the user interface */
        Screen::draw(ctx);
    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();
	    PlaylistEntry::Startup();
        
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

        /* scoped variables */ {
            nanogui::ref<PlaylistMakerApp> app = new PlaylistMakerApp();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << endl;
        printf(error_msg.c_str());
        throw e;
        //return -1;
    }

    return 0;
}
