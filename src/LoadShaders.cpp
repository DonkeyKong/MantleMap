#include "LoadShaders.hpp"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

#include "GLError.hpp"

bool GfxShader::checkShader()
{
  GLint isCompiled = 0;
  glGetShaderiv(Id, GL_COMPILE_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE)
  {
    GLint maxLength = 0;
    glGetShaderiv(Id, GL_INFO_LOG_LENGTH, &maxLength);
    
    // The maxLength includes the NULL character
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(Id, maxLength, &maxLength, &errorLog[0]);
    
    for (int i = 0; i < maxLength; i++)
    {
      if (errorLog[i] != '\0')
      std::cout << errorLog[i];
    }
    std::cout << std::endl;
    
    // Exit with failure.
    glDeleteShader(Id); // Don't leak the shader.
    return false;
  }
  return true;
}

bool GfxShader::LoadFragmentShader(const char* filename)
{
  assert(Src.size()==0);
  FILE* f = fopen(filename, "rb");
  assert(f);
  fseek(f,0,SEEK_END);
  int sz = ftell(f);
  fseek(f,0,SEEK_SET);
  Src.clear();
  Src.resize(sz+1);
  fread(Src.data(),1,sz,f);
  Src[sz] = 0; //null terminate it!
  fclose(f);

  //now create and compile the shader
  GLchar* sourcePtr = Src.data();
  Id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(Id, 1, (const GLchar**)&sourcePtr, 0);
  glCompileShader(Id);
  return checkShader();
}

bool GfxShader::LoadVertexShader(const char* filename)
{
  assert(Src.size()==0);
  FILE* f = fopen(filename, "rb");
  assert(f);
  fseek(f,0,SEEK_END);
  int sz = ftell(f);
  fseek(f,0,SEEK_SET);
  Src.clear();
  Src.resize(sz+1);
  fread(Src.data(),1,sz,f);
  Src[sz] = 0; //null terminate it!
  fclose(f);

  //now create and compile the shader
  GLchar* sourcePtr = Src.data();
  GlShaderType = GL_VERTEX_SHADER;
  Id = glCreateShader(GlShaderType);
  glShaderSource(Id, 1, (const GLchar**)&sourcePtr, 0);
  glCompileShader(Id);

  return checkShader();
}

GfxProgram::GfxProgram() 
{ 
  isLoaded = false;
}

GLuint GfxProgram::LoadShaders(const char * vertex_file_path, const char * fragment_file_path)
{
  VertexShader.LoadVertexShader(vertex_file_path);
  FragmentShader.LoadFragmentShader(fragment_file_path);
  Id = glCreateProgram();
  glAttachShader(Id, VertexShader.GetId());
  glAttachShader(Id, FragmentShader.GetId());
  glLinkProgram(Id);
  checkProgram(vertex_file_path, fragment_file_path);
  glUseProgram(GetId());
  print_if_glerror("Load shader program");
  isLoaded = true;
  return GetId();
}

bool GfxProgram::checkProgram(std::string vertName, std::string fragName)
{
  GLint isCompiled = 0;
  glGetProgramiv(Id, GL_LINK_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE)
  {
    std::cout << "Failed to compile program: " << vertName << " + " << fragName << std::endl;
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
    return false;
  }
  return true;
}

void GfxProgram::SetUniform(const char* name, float value)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform1f(loc, value);
    }
}

void GfxProgram::SetUniform(const char* name, bool value)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform1i(loc, value);
    }
}

void GfxProgram::SetUniform(const char* name, float value0, float value1)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform2f(loc, value0, value1);
    }
}

void GfxProgram::SetUniform(const char* name, float value0, float value1, float value2, float value3)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform4f(loc, value0, value1, value2, value3);
    }
}

void GfxProgram::SetUniform(const char* name, const Color& rgba)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform4f(loc, rgba.r, rgba.g, rgba.b, rgba.a);
    }
}

void GfxProgram::SetUniform(const char* name, const Position& xyz)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform3f(loc, xyz.x, xyz.y, xyz.z);
    }
}

void GfxProgram::SetUniform(const char* name, const Position2D& xy)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform2f(loc, xy.x, xy.y);
    }
}

void GfxProgram::SetUniform(const char* name, const TexCoord& uv)
{
    GLint loc = glGetUniformLocation(GetId(), name);
    if (loc != -1)
    {
       glUniform2f(loc, uv.u, uv.v);
    }
}

void GfxProgram::SetUniform(const char* name, int value)
{
  GLint loc = glGetUniformLocation(GetId(), name);
  if (loc != -1)
  {
    glUniform1i(loc, value);
  }
}

void GfxProgram::SetCameraFromPixelTransform(int mapWidth, int mapHeight)
{
  //glUseProgram(GetId());
  float xform[] = { 2.0f / (float)mapWidth , 0.0f, 0.0f, 0.0f,
                      0.0f,  -2.0f / (float)mapHeight, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      -1.0f, 1.0f, 0.0f, 1.0f};
  GLint loc = glGetUniformLocation(GetId(), "uCameraFromPixelTransform");
  if (loc != -1)
  {
    glUniformMatrix4fv(	loc, 1, GL_FALSE, xform);
  }
}

void GfxProgram::SetCameraFromPixelTransform(float mapWidth, float mapHeight, float x, float y, float scale)
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
  float sX = (scale / mapWidth) * 2.0f;
  float sY = -(scale / mapHeight) * 2.0f;
  float tX = (2.0 / mapWidth) * x - 1.0f;
  float tY = -(2.0 / mapHeight) * y + 1.0f;

  // float xform[] = {   sX,     0.0f,   0.0f,   tX,
  //                     0.0f,   sY,     0.0f,   tY,
  //                     0.0f,   0.0f,   1.0f,   0.0f,
  //                     0.0f,   0.0f,   0.0f,   1.0f };

  float xform[] = {   sX,   0.0f,   0.0f,   0.0f,
                    0.0f,     sY,   0.0f,   0.0f,
                    0.0f,   0.0f,   1.0f,   0.0f,
                      tX,     tY,   0.0f,   1.0f };

  GLint loc = glGetUniformLocation(GetId(), "uCameraFromPixelTransform");
  if (loc != -1)
  {
    glUniformMatrix4fv(	loc, 1, GL_FALSE, xform);
  }
}

GLuint LoadImageToTexture(std::string imagePath)
{
  int width, height;
  return LoadImageToTexture(imagePath, width, height);
}

GLuint LoadImageToTexture(std::string imagePath, int& imageWidth, int& imageHeight)
{
  ImageRGBA image = ImageRGBA::FromPngFile(imagePath);
  imageWidth = image.width();
  imageHeight = image.height();

  return LoadImageToTexture(image);
}

GLuint LoadImageToTexture(ImageRGBA& image)
{
  GLuint texID = 0;
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  return texID;
}

void LoadImageToTexture(ImageRGBA& image, GLuint existingTexture)
{
  glBindTexture(GL_TEXTURE_2D, existingTexture);
  glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
}

GfxProgram LoadGraphicsProgram(std::string vertShaderPath, std::string fragShaderPath)
{
  GfxProgram program;
  program.LoadShaders(  vertShaderPath.c_str(),
                        fragShaderPath.c_str());
  return program;
}

