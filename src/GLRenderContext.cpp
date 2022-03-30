#include "GLRenderContext.hpp"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <iostream>

#ifdef PI_HOST
#include <bcm_host.h>
#endif

#include "GLError.hpp"

static const EGLint attribute_list[] =
{
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_NONE
};
	
static const EGLint context_attributes[] = 
{
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
};

GLRenderContext::GLRenderContext(MapState& map) : _map(map)
{
  // Init the OpenGL context for this drawing
  initGL();
  
  print_if_glerror("RenderContext initGL");
}

GLRenderContext::~GLRenderContext()
{
}

void GLRenderContext::initGL()
{
	EGLint num_config;
  EGLBoolean result;

  #ifdef PI_HOST
  // Init the GPU hardware
	bcm_host_init();
  #endif
	
  // Get an EGL display connection
  GDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  assert(GDisplay!=EGL_NO_DISPLAY);
  print_if_glerror("Get EGL display");
  
  // Initialize the EGL display connection
  result = eglInitialize(GDisplay, NULL, NULL);
  assert(EGL_FALSE != result);
  print_if_glerror("Init EGL display");
	
	// Bind the OpenGL API so we can make calls to it
  result = eglBindAPI(EGL_OPENGL_ES_API);
  assert(EGL_FALSE != result);
  print_if_glerror("Bind OpenGL ES API");
    
  // Select an OpenGL configuration
	EGLConfig config;
  result = eglChooseConfig(GDisplay, attribute_list, &config, 1, &num_config);
  assert(EGL_FALSE != result);
  print_if_glerror("Choose config");
    
  // Bind the OpenGL ES API
  result = eglBindAPI(EGL_OPENGL_ES_API);
  assert(EGL_FALSE != result);
  print_if_glerror("Bind OpenGL ES API");
  
  // Create an OpenGL rendering context
	GContext = eglCreateContext(GDisplay, config, EGL_NO_CONTEXT, context_attributes);
	assert(GContext!=EGL_NO_CONTEXT);
	print_if_glerror("Create render context");
	
	GSurface = eglCreatePbufferSurface(GDisplay, config, NULL);
  eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);

	// Get info about the API
// 	std::cout << "Initializing OpenGL..." << std::endl;
// 	std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
// 	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
// 	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
// 	std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
// 	std::cout << "Supported Extensions: " << glGetString(GL_EXTENSIONS) << std::endl << std::flush;

	// Construct our render buffer
  FramebufferName = 0;
  glGenFramebuffers(1, &FramebufferName);
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  print_if_glerror("Generate framebuffer");

  // Create the render texture
  glGenTextures(1, &RenderedTexture);

  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, RenderedTexture);

  // Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0 ,GL_RGBA, _map.width, _map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  // Poor filtering. Needed !
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  print_if_glerror("Setup fb texture params");
}

void GLRenderContext::BeginDraw()
{
  // Sometimes we have a context for the display
  // make the render contenxt current here just in case
  eglMakeCurrent(GDisplay, GSurface, GSurface, GContext);

  // Set "RenderedTexture" as color attachement #0
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RenderedTexture, 0);

  // Bind to the frame buffer
  glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
  glViewport(0,0,_map.width,_map.height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
    
  // Clear the whole buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );
}
