#include "DisplayDevice.hpp"
#include <exception>

#ifdef LED_PANEL_SUPPORT
#include "led-matrix.h"
#include "pixel-mapper.h"
#include "content-streamer.h"
using rgb_matrix::GPIO;
using rgb_matrix::Canvas;
using rgb_matrix::FrameCanvas;
using rgb_matrix::RGBMatrix;
using rgb_matrix::StreamReader;

struct DisplayDevice::Impl
{
    MapState& map;
    RGBMatrix* matrix;
    FrameCanvas* offscreen_canvas;
    uint8_t* CPUTextureCache;

    Impl(MapState& map) : map(map)
    {
        // Prepare matrix
        matrix = CreateMatrixFromOptions(matrixParams, runtimeParams);
        if (matrix == nullptr)
        {
            throw std::runtime_error("LED display init failed!");
        }
        
        // Create our double buffering canvas
        offscreen_canvas = matrix->CreateFrameCanvas();

        // Create the CPUTextureCache in RGBA format
        CPUTextureCache = (uint8_t*) malloc( _map.width * _map.height * 4 * sizeof(uint8_t) );
    }

    ~Impl()
    {
        free(CPUTextureCache);
        matrix->Clear();
        delete matrix;
    }

    DrawFromGLFramebuffer()
    {
        // Copy out to CPU
        glReadPixels(0,0, map.width, map.height, GL_RGBA, GL_UNSIGNED_BYTE,  CPUTextureCache);
        
        // Copy into offscreen LED Matrix buffer
        uint8_t* img =  CPUTextureCache;
        for (int y=0; y < map.height; y++)
        {
            for (int x=0; x < map.width; x++)
            {
                offscreen_canvas->SetPixel(x, (map.height-1)-y, img[0],img[1], img[2]);
                img += 4;
            }
        }

        offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
    }
};

DisplayDevice::DisplayDevice(MapState& map)
{
    pImpl_ = std::make_unique<Impl>(map);
}

DisplayDevice::Update()
{
	pImpl_->DrawFromGLFramebuffer();
}

DisplayDevice::Clear()
{
    pImpl_->matrix->Clear();
}

#else

// Dummy display device that does nothing
struct DisplayDevice::Impl {};
DisplayDevice::DisplayDevice(MapState& map) {}
DisplayDevice::~DisplayDevice() {}
void DisplayDevice::Update() {}
void DisplayDevice::Clear() {}

#endif