#include "ImageView.hpp"
#include "GLError.hpp"

GfxProgram ImageView::_program;
GLint ImageView::_vertexAttrib;
GLint ImageView::_textureAttrib;

ImageView::ImageView(MapState& map) : SceneElement(map)
{    
  texture = 0;
  dirty = false;
  x = 0; y = 0; scale = 1;
  r = 1; g = 1; b = 1; a = 1;
}

void ImageView::initGL()
{  
  if (!_program.isLoaded)
  {
    print_if_glerror("before initGL for ImageView");
    // Load and compile the shaders into a glsl program
    _program.LoadShaders(map.GetResourcePath("imagevertshader.glsl").c_str(),
                         map.GetResourcePath("imagefragshader.glsl").c_str());
    _vertexAttrib = glGetAttribLocation(_program.GetId(), "aVertex");
    print_if_glerror("get vertex coord ImageView");
    _textureAttrib = glGetAttribLocation(_program.GetId(), "aTexCoord");
    print_if_glerror("get tex coord ImageView");
    _program.SetCameraFromPixelTransform(map.width, map.height, x ,y, scale);
    print_if_glerror("set transform for ImageView");
  }
}

ImageView::~ImageView()
{

}

void ImageView::SetImage(ImageRGBA image)
{
  this->image = std::move(image);
  dirty = true;
}

void ImageView::SetPosition(float x, float y)
{
  this->x = x;
  this->y = y;
}

int ImageView::GetWidth()
{
  return image.width() * scale;
}

int ImageView::GetHeight()
{
  return image.height() * scale;
}

void ImageView::SetColor(float r, float g, float b, float a)
{
  this->r = r;
  this->g = g;
  this->b = b;
  this->a = a;
}

void ImageView::SetScale(float scale)
{
  this->scale = scale;
}

void ImageView::drawInternal()
{
  if (dirty)
  {
    if (texture != 0)
    {
      glDeleteTextures(1, &texture);
      texture = 0;
      print_if_glerror("delete texture for ImageView");
    }


    #ifdef NPOT_TEXTURE_SUPPORT
    float maxU = 1.0f;
    float maxV = 1.0f;
    #else
    // Something subtle is wrong here causing the texture mapping to be off by a pixel or two
    image.PadToPowerOfTwo();
    float maxU = 1.0f - ((float)image.padW() / (float)image.width());
    float maxV = 1.0f - ((float)image.padH() / (float)image.height());
    #endif

    texture = LoadImageToTexture(image);
    print_if_glerror("Load texture for ImageView");

    // Create the mesh for the image view
    //       X                  Y                          Z       U       V
    mesh = { 0.0f,                0.0f,                   0.0f,   0.0f,   0.0f,
            (float)image.width(), 0.0f,                   0.0f,   maxU,   0.0f, 
            0.0f,                 (float)image.height(),  0.0f,   0.0f,   maxV,
            (float)image.width(), (float)image.height(),  0.0f,   maxU,   maxV  };

    dirty = false;
  }

  if (mesh.size() > 0 && image.height() > 0)
  {
    // Draw the image
	  glUseProgram(_program.GetId());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    print_if_glerror("Texture bind for ImageView");

    // Tell our shader which units to look for each texture on
    _program.SetUniform("uTexture", 0);
    _program.SetUniform("uColor", r, g, b, a);
    _program.SetUniform("uTextureSize", (float)image.width(), (float)image.height());
    _program.SetCameraFromPixelTransform(map.width, map.height, x ,y, scale);
    print_if_glerror("Shader setup for ImageView");

    glVertexAttribPointer(
                  _vertexAttrib,      // The attribute ID
                  3,                  // size
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  5*sizeof(float),                  // stride
                  mesh.data()         // underlying data
          );

    glEnableVertexAttribArray ( _vertexAttrib );
    
    glVertexAttribPointer(
                        _textureAttrib, // The attribute ID
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        5*sizeof(float),   // stride
                        mesh.data()+3      // underlying data
                );
                
    glEnableVertexAttribArray(_textureAttrib);

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    print_if_glerror("DrawArrays for ImageView");
  }
}
