#pragma once

#include "EGL/egl.h"
#include "EGL/eglplatform.h"
#include "GLES2/gl2.h"
#include "EGL/eglext.h"

class GLRenderContext
{
 public:
    GLRenderContext();
    ~GLRenderContext();
    
    // Draw the specified scene to the RGB matrix
    void BeginDraw();

  private:
    void initGL();
    
    EGLDisplay GDisplay;
    EGLContext GContext;
    EGLSurface GSurface;
    GLuint FramebufferName;
    GLuint RenderedTexture;
};