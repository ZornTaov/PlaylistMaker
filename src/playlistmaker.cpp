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
#include <iostream>
#include <string>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>

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

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication() : nanogui::Screen(Eigen::Vector2i(1280, 720), "NanoGUI Test") {
        using namespace nanogui;
        glfwSetJoystickCallback(joystickCallback);
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
        b->setChangeCallback([](bool state) { cout << "Toggled" << state << endl; });
        
        b = layer->add<Button>();
        b->setCallback([] { cout << "pushed!" << endl; });
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

    static void joystickCallback(int jid, int event)
    {
        if (event == GLFW_CONNECTED)
        {
            printf("Joystick %d connected\n", jid);
            if (glfwJoystickIsGamepad(jid))
                printf("Joystick %d is gamepad: \"%s\"\n", jid, glfwGetGamepadName(jid));
        }
        else if (event == GLFW_DISCONNECTED)
            printf("Joystick %d disconnected\n", jid);
    }
    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        
        // Gamepad
        GLFWgamepadstate gamepad = {};
        if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
        {
            // Gamepad not available, so let's fake it with keyboard
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  = glfwGetKey(window, GLFW_KEY_LEFT);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = glfwGetKey(window, GLFW_KEY_RIGHT);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]    = glfwGetKey(window, GLFW_KEY_UP);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  = glfwGetKey(window, GLFW_KEY_DOWN);
            gamepad.buttons[GLFW_GAMEPAD_BUTTON_START]      = glfwGetKey(window, GLFW_KEY_ESCAPE);
        }

        // Exit by pressing Start (aka Plus)
        if (gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            return true;
        }
        /* Draw the user interface */
        Screen::draw(ctx);
    }
};

int main(int /* argc */, char ** /* argv */) {
    try {
        nanogui::init();

        /* scoped variables */ {
            nanogui::ref<ExampleApplication> app = new ExampleApplication();
            app->drawAll();
            app->setVisible(true);
            nanogui::mainloop();
        }

        nanogui::shutdown();
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << endl;
        return -1;
    }

    return 0;
}
