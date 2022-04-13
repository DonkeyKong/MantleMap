#pragma once

#include "GLES2/gl2.h"
#include <string>
#include <vector>

struct ShaderFeature
{
    static const std::string VertexColor; // FEATURE_VERTEX_COLOR
    static const std::string Texture;    // FEATURE_TEXTURE
    static const std::string PixelSnap;   // FEATURE_PIXEL_SNAP
};

enum class ShaderType : GLenum
{
    VertexShader = GL_VERTEX_SHADER,
    FragmentShader = GL_FRAGMENT_SHADER,
};

class GfxShader
{
public:
	GfxShader(const std::string& path, ShaderType shaderType, const std::vector<std::string>& features);
	~GfxShader();
	GLuint GetId() const;
    std::string GetPath() const;
    std::string GetSrc() const;

private:
    std::string Path;
	std::string Src;
	GLuint Id{0};
	ShaderType shaderType;
    void checkShader();
};