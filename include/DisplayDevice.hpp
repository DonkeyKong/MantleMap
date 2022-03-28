#pragma once

#include <memory>
#include "MapState.hpp"

// Display device is an abstraction that allows our framebuffer to be drawn to
// an array of LED panels, a window on a PC, or any other image output

class DisplayDevice final
{
    public:
        // Display devices must be constructed in the main thread
        DisplayDevice(MapState& map);

        ~DisplayDevice();

        // Use glReadPixels to get the rendered image and display it
        // May wait for some kind of framebuffer sync
        void Update(); 

        // Clear the display and if possible, enter a low power state
        void Clear();

        // ProcessEvents must be called from the main thread and
        // if it returns false, that means the window has closed
        // and the program should exit
        bool ProcessEvents();

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl_;
};
