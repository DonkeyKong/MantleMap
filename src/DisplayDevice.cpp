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
        // Mantle map parameters
        int cols = 64;
        int rows = 32;
        int chainLength = 3;
        int parallelLength = 3;
        int slowdown = 2;
        
        // Move params into structures
        RGBMatrix::Options matrixParams;
        //std::string hardware_mapping = "regular";
        matrixParams.rows = rows;
        matrixParams.cols = cols;
        matrixParams.chain_length = chainLength;
        matrixParams.hardware_mapping = "regular";
        matrixParams.parallel = parallelLength;
        matrixParams.pwm_lsb_nanoseconds = 200;
        matrixParams.brightness = 100;
        matrixParams.pwm_bits = 6;
        rgb_matrix::RuntimeOptions runtimeParams;
        runtimeParams.do_gpio_init = true;
        runtimeParams.gpio_slowdown = slowdown;

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

bool DisplayDevice::ProcessEvents()
{
    // The LED panel does not have events!
    return true;
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
#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

#include "LoadShaders.hpp"

#include <util/OSWindow.h>
#include <thread>
#include <filesystem>

static const EGLint attribute_list[] =
{
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
};
    
static const EGLint context_attributes[] = 
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

// This display device opens an OS native window and renders a preview to the display
struct DisplayDevice::Impl 
{
    MapState& map;
    OSWindow* window;
    bool running = true;
    EGLSurface surface;
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;
    std::vector<uint8_t> CPUTextureCache;
    GfxProgram program;
    GLuint texture;
    GLint vertexAttrib;
    GLint coordinateAttrib;
    std::vector<float> mesh;

    std::string GetResourcePath(std::string resourceName)
    {
        auto filePath = std::filesystem::path(map.sceneResourcePath) / "Shared" / resourceName;
        if (std::filesystem::exists(filePath))
        {
            return filePath;
        }
        return std::string();
    }

    Impl(MapState& map) : map(map), CPUTextureCache(map.width * map.height * 4 * sizeof(uint8_t))
    {
        // Create the OS window
        window = OSWindow::New();
        if (!window->initialize(map.defaultScene, map.width * 4, map.height * 4))
        {
            throw std::runtime_error("Couldn't open OS window!");
        }
        window->setVisible(true);
        window->setPosition(100, 100);

        // Get the EGL display, config, and create the surface
        EGLint num_config;
        EGLBoolean result;
        
        display = eglGetDisplay(window->getNativeDisplay());
        result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
        if (result == EGL_FALSE)
        {
            throw std::runtime_error("Couldn't create config!");
        }
        
        surface = eglCreateWindowSurface(display, config, window->getNativeWindow(), nullptr);
        if (surface == EGL_NO_SURFACE)
        {
            throw std::runtime_error("Couldn't create surface!");
        }

        // Create an OpenGL rendering context
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
        if (context == EGL_NO_CONTEXT)
        {
            throw std::runtime_error("Couldn't create context!");
        }

        // Switch contexts to this display
        eglMakeCurrent(display, surface, surface, context);

        // Load and compile the image display shaders into a glsl program
        program = LoadGraphicsProgram(GetResourcePath("imagevertshader.glsl"), 
                                      GetResourcePath("imagefragshader.glsl"));
        program.SetCameraFromPixelTransform(map.width,map.height);
        vertexAttrib = glGetAttribLocation(program.GetId(), "aVertex");
        coordinateAttrib = glGetAttribLocation(program.GetId(), "aTexCoord");

        // Create a texture for the display image
        texture = 0;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, map.width, map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CPUTextureCache.data());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        // Create the mesh for the image render
        //       X                  Y                   Z       U       V
        mesh = { 0.0f,              0.0f,               0.0f,   0.0f,   1.0f,
                (float)map.width,   0.0f,               0.0f,   1.0f,   1.0f, 
                0.0f,               (float)map.height,  0.0f,   0.0f,   0.0f,
                (float)map.width,   (float)map.height,  0.0f,   1.0f,   0.0f  };
    }

    void update()
    {
        // Copy out render to CPU
        glReadPixels(0,0, map.width, map.height, GL_RGBA, GL_UNSIGNED_BYTE,  CPUTextureCache.data());

        // Switch contexts to this display
        eglMakeCurrent(display, surface, surface, context);

        // Push the new render into the texture
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, map.width, map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, CPUTextureCache.data());

        // Set the viewport
        float winRatio = (float)window->getWidth() /  (float)window->getHeight();
        float mapRatio = (float)map.width /  (float)map.height;

        if ( winRatio > mapRatio ) // Wider than normal
        {
            GLsizei width = mapRatio * window->getHeight();
            GLint offset =  (window->getWidth() - width) / 2;
            glViewport(offset, 0, width, window->getHeight());
        }
        else
        {
            GLsizei height = window->getWidth() /  mapRatio;
            GLint offset =  (window->getHeight() - height) / 2;
            glViewport(0, offset, window->getWidth(), height);
        }

        glClearColor(0.1,0.1,0.1,1);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the image
	    glUseProgram(program.GetId());
        glActiveTexture(GL_TEXTURE0);

        // Tell our shader which units to look for each texture on
        program.SetUniform("uTexture", 0);
        program.SetUniform("uLocation", 0.0f, 0.0f);
        program.SetUniform("uColor", 1.0f, 1.0f, 1.0f, 1.0f);
        program.SetUniform("uTextureSize", (float)map.width, (float)map.height);

        glVertexAttribPointer(
                      vertexAttrib,      // The attribute ID
                      3,                  // size
                      GL_FLOAT,           // type
                      GL_FALSE,           // normalized?
                      5*sizeof(float),                  // stride
                      mesh.data()         // underlying data
              );

        glEnableVertexAttribArray ( vertexAttrib );
        
        glVertexAttribPointer(
                            coordinateAttrib, // The attribute ID
                            2,                  // size
                            GL_FLOAT,           // type
                            GL_FALSE,           // normalized?
                            5*sizeof(float),   // stride
                            mesh.data()+3      // underlying data
                    );
                    
        glEnableVertexAttribArray(coordinateAttrib);

        // Draw the triangles!
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


        eglSwapBuffers(display, surface);
    }

    bool processWindowEvents()
    {
        if (running)
        {
            Event event;
            while (window->popEvent(&event))
            {
                if (event.Type == Event::EVENT_CLOSED)
                {
                    running = false;
                    break;
                }

                if (event.Type == Event::EVENT_KEY_PRESSED)
                {
                    switch (event.Key.Code)
                    {
                        case KEY_ESCAPE:
                            running = false;
                            break;
                        default:
                            break;
                    }
                    //window->resize(width, height);
                    //window->setPosition(x, y);
                }
            }
            
            window->messageLoop();
        }

        return running;
    }

    ~Impl()
    {
        running = false;
        if (window != nullptr && window->valid())
            window->destroy();
    }
};

DisplayDevice::DisplayDevice(MapState& map) 
{
    pImpl_ = std::make_unique<Impl>(map);
}

DisplayDevice::~DisplayDevice() 
{

}

bool DisplayDevice::ProcessEvents()
{
    return pImpl_->processWindowEvents();
}

void DisplayDevice::Update() 
{
    pImpl_->update();
}

void DisplayDevice::Clear() 
{
    // no such thing
}

#endif