#include "GfxShader.hpp"
#include "GLError.hpp"

#include <fmt/format.h>

#include <fstream>
#include <sstream>

const std::string ShaderFeature::VertexColor = "FEATURE_VERTEX_COLOR";
const std::string ShaderFeature::Texture = "FEATURE_TEXTURE";
const std::string ShaderFeature::PixelSnap = "FEATURE_PIXEL_SNAP";

GfxShader::GfxShader(const std::string& path, ShaderType shaderType, const std::vector<std::string>& features)
{
    Path = path;
    std::stringstream buffer;

    // Set the defines
    for (const auto& feature : features)
    {
        buffer << "#define " << feature << std::endl;
    }

    // Read the source file and dump it to the buffer
    {
        std::ifstream t(path);
        buffer << t.rdbuf();
    }

    // Store the buffer so we can refer back to it
    // (and send it to OGL)
    Src = buffer.str();

    // Create and compile the shader
    GLchar* sourcePtr = Src.data();
    Id = glCreateShader(static_cast<GLenum>(shaderType));
    glShaderSource(Id, 1, (const GLchar**)&sourcePtr, 0);
    glCompileShader(Id);
    checkShader();
}

GfxShader::~GfxShader()
{
    if (Id != 0)
        glDeleteShader(Id);
}

std::string GfxShader::GetPath() const
{
    return Path;
}

std::string GfxShader::GetSrc() const
{
    return Src;
}

GLuint GfxShader::GetId() const
{ 
    return Id; 
}

void GfxShader::checkShader()
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
    Id = 0;
    throw std::runtime_error(fmt::format("Shader {} was invalid or failed to compile!", Path));
  }
}