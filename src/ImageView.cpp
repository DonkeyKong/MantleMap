#include "ImageView.hpp"
#include "GLError.hpp"

std::unique_ptr<GfxProgram> ImageView::_program;

ImageView::ImageView(ConfigService& map) : SceneElement(map)
{    
  texture = 0;
  dirty = false;
  x = 0; y = 0; scale = 1;
}

void ImageView::initGL()
{  
  if (!_program)
  {
    print_if_glerror("before initGL for ImageView");
    // Load and compile the shaders into a glsl program
    _program = loadProgram( "vertshader.glsl", "fragshader.glsl",
                            {
                                    ShaderFeature::PixelSnap,
                                    ShaderFeature::Texture
                            });
    print_if_glerror("set transform for ImageView");
  }
}

ImageView::~ImageView()
{

}

void ImageView::SetImage(std::shared_ptr<ImageRGBA> image)
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
  return image->width() * scale;
}

int ImageView::GetHeight()
{
  return image->height() * scale;
}

void ImageView::SetColor(float r, float g, float b, float a)
{
    tint = {r,g,b,a};
}

void ImageView::SetScale(float scale)
{
  this->scale = scale;
}

void ImageView::drawInternal()
{
  if (dirty)
  {
    texture = std::make_unique<GfxTexture>(*image);
    print_if_glerror("Load texture for ImageView");

    // Create the mesh for the image view
    //       X                  Y                          Z       U       V
    mesh = { 0.0f,                0.0f,                   0.0f,   0.0f,   0.0f,
            (float)texture->GetWidth(), 0.0f,                   0.0f,   1.0f,   0.0f, 
            0.0f,                       (float)texture->GetHeight(),  0.0f,   0.0f,   1.0f,
            (float)texture->GetWidth(), (float)texture->GetHeight(),  0.0f,   1.0f,   1.0f  };

    dirty = false;
  }

  if (mesh.size() > 0 && texture->GetHeight() > 0)
  {
    // Draw the image
    _program->Use();

    // Tell our shader which units to look for each texture on
    _program->SetTexture0(*texture);
    print_if_glerror("Texture bind for ImageView");
    _program->SetTint(tint);
    _program->SetModelTransform(Transform3D::FromTranslationAndScale(x,y,0,scale));
    print_if_glerror("Shader setup for ImageView");

    glVertexAttribPointer(
                  _program->Attrib("aPosition"),      // The attribute ID
                  3,                  // size
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  5*sizeof(float),                  // stride
                  mesh.data()         // underlying data
          );

    glEnableVertexAttribArray ( _program->Attrib("aPosition") );
    
    glVertexAttribPointer(
                        _program->Attrib("aTexCoord"), // The attribute ID
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        5*sizeof(float),   // stride
                        mesh.data()+3      // underlying data
                );
                
    glEnableVertexAttribArray(_program->Attrib("aTexCoord"));

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    print_if_glerror("DrawArrays for ImageView");
  }
}
