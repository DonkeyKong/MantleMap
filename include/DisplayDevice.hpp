#pragma once

#include "ConfigService.hpp"
#include "InputButton.hpp"

#include <memory>
#include <sigslot/signal.hpp>

// Display device is an abstraction that allows our framebuffer to be drawn to
// an array of LED panels, a window on a PC, or any other image output

class DisplayDevice final
{
    public:
        // Display devices must be constructed in the main thread
        DisplayDevice();

        ~DisplayDevice();

        // Use glReadPixels to get the rendered image and display it
        // May wait for some kind of framebuffer sync
        void Update(); 

        // Clear the display and if possible, enter a low power state
        void Clear();

        // Gets an input button, if any, provided by the display
        // Pointer should be good for the display's lifetime
        // Returns nullptr if there is no button
        InputButton* GetInputButton();

        // Sometimes the display disconnects from the system. When this
        // happens we should probably exit right away
        sigslot::signal<> OnDisconnect;

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl_;
};
