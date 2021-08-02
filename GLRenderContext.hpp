#ifndef GLRENDERCONTEXT_HPP
#define GLRENDERCONTEXT_HPP

#include "Scene.hpp"
#include "MapState.hpp"

#include "led-matrix.h"

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

class GLRenderContext
{
 public:
    GLRenderContext(MapState& map);
    ~GLRenderContext();
    
    // Draw the specified scene to the RGB matrix
    void BeginDraw();
    void FinishDraw(rgb_matrix::FrameCanvas* canvas);

  private:
    void initGL();
    MapState& _map;
    
    EGLDisplay GDisplay;
    EGLContext GContext;
    EGLSurface GSurface;
    GLuint FramebufferName;
    GLuint RenderedTexture;
    uint8_t* CPUTextureCache;
};

#endif
