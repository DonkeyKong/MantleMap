#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <assert.h>

#include "LoadShaders.hpp"

#include <Magick++.h>
#include <magick/image.h>

using namespace Magick;

#define check() assert(glGetError() == 0)

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
        //cheeky bit of code to read the whole file into memory
        assert(!Src);
        FILE* f = fopen(filename, "rb");
        assert(f);
        fseek(f,0,SEEK_END);
        int sz = ftell(f);
        fseek(f,0,SEEK_SET);
        Src = new GLchar[sz+1];
        fread(Src,1,sz,f);
        Src[sz] = 0; //null terminate it!
        fclose(f);

        //now create and compile the shader
        Id = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(Id, 1, (const GLchar**)&Src, 0);
        glCompileShader(Id);
        return checkShader();
}

bool GfxShader::LoadVertexShader(const char* filename)
{

        //cheeky bit of code to read the whole file into memory
        assert(!Src);
        FILE* f = fopen(filename, "rb");
        assert(f);
        fseek(f,0,SEEK_END);
        int sz = ftell(f);
        fseek(f,0,SEEK_SET);
        Src = new GLchar[sz+1];
        fread(Src,1,sz,f);
        Src[sz] = 0; //null terminate it!
        fclose(f);

        //now create and compile the shader
        GlShaderType = GL_VERTEX_SHADER;
        Id = glCreateShader(GlShaderType);
        glShaderSource(Id, 1, (const GLchar**)&Src, 0);
        glCompileShader(Id);

        return checkShader();
}

GfxProgram::GfxProgram() 
{ 
  isLoaded = false;
}

GLuint GfxProgram::LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
  VertexShader.LoadVertexShader(vertex_file_path);
  FragmentShader.LoadFragmentShader(fragment_file_path);
  Id = glCreateProgram();
  glAttachShader(Id, VertexShader.GetId());
  glAttachShader(Id, FragmentShader.GetId());
  glLinkProgram(Id);
  checkProgram();
  glUseProgram(GetId());
  check();
  isLoaded = true;
  return GetId();
}

bool GfxProgram::checkProgram()
{
  GLint isCompiled = 0;
  glGetProgramiv(Id, GL_LINK_STATUS, &isCompiled);
  if(isCompiled == GL_FALSE)
  {
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

GLuint LoadImageToTexture(std::string imagePath)
{
  int width, height;
  return LoadImageToTexture(imagePath, width, height);
}

GLuint LoadImageToTexture(std::string imagePath, int& imageWidth, int& imageHeight)
{
  // Read the map texture image from disk
  Image image;
  imageWidth = -1;
  imageHeight = -1;
  GLuint texID = 0;
  
  try
  {
    // Read a file into image object
    image.read( imagePath );
  }
  catch( Magick::Exception &error_ )
  {
    std::cout << "Caught exception: " << error_.what() << std::endl;
    return texID;
  }
  
  imageWidth = image.columns();
  imageHeight = image.rows();
  std::vector<unsigned char> data(imageWidth*imageHeight*4);
  image.write(0,0,imageWidth,imageHeight,"RGBA", Magick::CharPixel, &data[0]);
  
  glGenTextures(1, &texID);
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0 , GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  
  return texID;
}

GfxProgram LoadGraphicsProgram(std::string vertShaderPath, std::string fragShaderPath)
{
  GfxProgram program;
  program.LoadShaders(  vertShaderPath.c_str(),
                        fragShaderPath.c_str());
  return program;
}

