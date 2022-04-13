#pragma once

#include "ConfigService.hpp"
#include "GfxShader.hpp"
#include "GfxTexture.hpp"
#include "Attributes.hpp"

#include "GLES2/gl2.h"

#include <string>
#include <vector>

class GfxProgram
{
public:
    GfxProgram() = delete;
    GfxProgram(const GfxProgram&) = delete;
    GfxProgram(GfxProgram&&) = delete;
	GfxProgram(ConfigService& config, const std::string& vertPath, const std::string& fragPath, const std::vector<std::string>& features);
	~GfxProgram();

	void SetUniform(const char* name, bool value);
	void SetUniform(const char* name, float value);
	void SetUniform(const char* name, int value);
	void SetUniform(const char* name, float value0, float value1);
	void SetUniform(const char* name, float value0, float value1, float value2, float value3);
	void SetUniform(const char* name, const Color& rgba);
	void SetUniform(const char* name, const Vec3& xyz);
	void SetUniform(const char* name, const Vec2& xy);
	void SetUniform(const char* name, const TexCoord& uv);

    // All shaders support these
    void SetTint(const Color& rgba);
	void SetModelTransform(float tX, float tY, float scale);
    void SetModelTransform(float tX, float tY, float scaleX, float scaleY);
    void SetTexture0(const GfxTexture& texture);
    void SetTexture1(const GfxTexture& texture);
    void SetTexture2(const GfxTexture& texture);
    void SetTexture3(const GfxTexture& texture);

    // Make this shader program active for binding and drawing
    void Use();

    GLint Attrib(const std::string& attribName);

private:
	std::unique_ptr<GfxShader> VertexShader;
	std::unique_ptr<GfxShader> FragmentShader;
    std::unordered_map<std::string, GLint> attribCache;
	GLuint Id{0};
    ConfigService& config;
    void setCameraFromPixelTransform();
    void checkProgram();
};

