#include "GfxProgram.hpp"
#include "GLError.hpp"

GfxProgram::GfxProgram(ConfigService& config, const std::string& vertPath, const std::string& fragPath, const std::vector<std::string>& features) :
    config(config)
{
    VertexShader = std::make_unique<GfxShader>(vertPath, ShaderType::VertexShader, features);
    FragmentShader = std::make_unique<GfxShader>(fragPath, ShaderType::FragmentShader, features);

    Id = glCreateProgram();
    glAttachShader(Id, VertexShader->GetId());
    glAttachShader(Id, FragmentShader->GetId());
    glLinkProgram(Id);
    checkProgram();
    glUseProgram(Id);
    print_if_glerror("Load shader program");

    // Set some default values in uniforms that will allow rendering
    // something sane if not setup elsewhere
    SetTint({1,1,1,1});
    SetModelTransform(0,0,1);
    setCameraFromPixelTransform();
}

GfxProgram::~GfxProgram()
{
    if (Id != 0)
    {
        glDeleteProgram(Id);
    }
    Id = 0;
}

void GfxProgram::Use()
{
    // Select our shader program
	glUseProgram(Id);
}

GLint GfxProgram::Attrib(const std::string& attribName)
{
    const auto& attrib = attribCache.find(attribName);
    if (attrib == attribCache.end())
    {
        auto attribLoc = glGetAttribLocation(Id, attribName.c_str());
        attribCache.emplace(attribName, attribLoc);
        return attribLoc;
    }
    return attrib->second;
}

void GfxProgram::SetTint(const Color& rgba)
{
    SetUniform("uTint", rgba);
}

void GfxProgram::SetTexture0(const GfxTexture& texture)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture.GetId());
  SetUniform("uTexture", 0);
  SetUniform("uTextureSize", (float)texture.GetWidth(), (float)texture.GetHeight());
}

void GfxProgram::SetTexture1(const GfxTexture& texture)
{
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture.GetId());
  SetUniform("uTexture1", 1);
  SetUniform("uTextureSize1", (float)texture.GetWidth(), (float)texture.GetHeight());
}

void GfxProgram::SetTexture2(const GfxTexture& texture)
{
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture.GetId());
  SetUniform("uTexture2", 2);
  SetUniform("uTextureSize2", (float)texture.GetWidth(), (float)texture.GetHeight());
}

void GfxProgram::SetTexture3(const GfxTexture& texture)
{
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, texture.GetId());
  SetUniform("uTexture3", 3);
  SetUniform("uTextureSize3", (float)texture.GetWidth(), (float)texture.GetHeight());
}

void GfxProgram::setCameraFromPixelTransform()
{

  // Camera space is like this:
  //////////////////////////////
  //           (0,1)          //
  //             |            //
  //             |            //
  // (-1,0)----(0,0)----(1,0) //
  //             |            //
  //             |            //
  //           (0,-1)         //
  //////////////////////////////

  // But our app acts like everything is in pixel space on the map
  // This transform aims to paper over this inconvenient mismatch

  float xform[] = { 2.0f / (float)config.width , 0.0f, 0.0f, 0.0f,
                      0.0f,  -2.0f / (float)config.height, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      -1.0f, 1.0f, 0.0f, 1.0f};
  GLint loc = glGetUniformLocation(Id, "uCameraFromPixelTransform");
  if (loc != -1)
  {
    glUniformMatrix4fv(	loc, 1, GL_FALSE, xform);
  }
}

void GfxProgram::SetModelTransform(float tX, float tY, float s)
{
  SetModelTransform(tX, tY, s, s);
}

void GfxProgram::SetModelTransform(float tX, float tY, float sX, float sY)
{
  float xform[] = {  sX ,   0.0f,   0.0f,   0.0f,
                    0.0f,    sY ,   0.0f,   0.0f,
                    0.0f,   0.0f,   1.0f,   0.0f,
                      tX,     tY,   0.0f,   1.0f };

  GLint loc = glGetUniformLocation(Id, "uPixelFromModelTransform");
  if (loc != -1)
  {
    glUniformMatrix4fv(	loc, 1, GL_FALSE, xform);
  }
}

// GLuint GfxProgram::GetId() const
// { 
//     return Id; 
// }

void GfxProgram::checkProgram()
{
  GLint isCompiled = 0;
  glGetProgramiv(Id, GL_LINK_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE)
  {
    std::cout << "Failed to compile program: " << VertexShader->GetPath() << " + " << FragmentShader->GetPath() << std::endl;
    GLint maxLength = 0;
    glGetProgramiv(Id, GL_INFO_LOG_LENGTH, &maxLength);
    
    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetProgramInfoLog(Id, maxLength, &maxLength, &errorLog[0]);
    
    for (int i = 0; i < maxLength; i++)
    {
      if (errorLog[i] != '\0')
          std::cout << errorLog[i];
   }
   std::cout << std::endl;
    
    // Exit with failure.
    glDeleteProgram(Id); // Don't leak the program.
    Id = 0;

    throw std::runtime_error("Error assembling shader program!");
  }
}

void GfxProgram::SetUniform(const char* name, float value)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform1f(loc, value);
    }
}

void GfxProgram::SetUniform(const char* name, bool value)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform1i(loc, value);
    }
}

void GfxProgram::SetUniform(const char* name, float value0, float value1)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform2f(loc, value0, value1);
    }
}

void GfxProgram::SetUniform(const char* name, float value0, float value1, float value2, float value3)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform4f(loc, value0, value1, value2, value3);
    }
}

void GfxProgram::SetUniform(const char* name, const Color& rgba)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform4f(loc, rgba.r, rgba.g, rgba.b, rgba.a);
    }
}

void GfxProgram::SetUniform(const char* name, const Position& xyz)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform3f(loc, xyz.x, xyz.y, xyz.z);
    }
}

void GfxProgram::SetUniform(const char* name, const Position2D& xy)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform2f(loc, xy.x, xy.y);
    }
}

void GfxProgram::SetUniform(const char* name, const TexCoord& uv)
{
    GLint loc = glGetUniformLocation(Id, name);
    if (loc != -1)
    {
       glUniform2f(loc, uv.u, uv.v);
    }
}

void GfxProgram::SetUniform(const char* name, int value)
{
  GLint loc = glGetUniformLocation(Id, name);
  if (loc != -1)
  {
    glUniform1i(loc, value);
  }
}
