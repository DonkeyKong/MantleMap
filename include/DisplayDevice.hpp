#pragma once

#include <memory>
#include "ConfigService.hpp"
#include "InputButton.hpp"

// Display device is an abstraction that allows our framebuffer to be drawn to
// an array of LED panels, a window on a PC, or any other image output

class DisplayDevice final
{
    public:
        // Display devices must be constructed in the main thread
        DisplayDevice(ConfigService& map);

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

    private:
        struct Impl;
        std::unique_ptr<Impl> pImpl_;
};
