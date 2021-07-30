#ifndef LOADSHADERS_HPP
#define LOADSHADERS_HPP

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <string>

class GfxShader
{
	GLchar* Src;
	GLuint Id;
	GLuint GlShaderType;
  bool checkShader();

public:

	GfxShader() : Src(NULL), Id(0), GlShaderType(0) {}
	~GfxShader() { if(Src) delete[] Src; }

	bool LoadVertexShader(const char* filename);
	bool LoadFragmentShader(const char* filename);
	GLuint GetId() { return Id; }
};

class GfxProgram
{
	GfxShader VertexShader;
	GfxShader FragmentShader;
	GLuint Id;

public:

	GfxProgram();
	~GfxProgram() {}
	GLuint LoadShaders(const char * vertex_file_path, const char * fragment_file_path);
	GLuint GetId() { return Id; }
	bool isLoaded;
  bool checkProgram();
  void SetUniform(const char* name, bool value);
	void SetUniform(const char* name, float value);
	void SetUniform(const char * name, int value);
	void SetUniform(const char* name, float value0, float value1);
  void SetUniform(const char* name, float value0, float value1, float value2, float value3);
  void SetCameraFromPixelTransform(int mapWidth, int mapHeight);
};

// Some OpenGL utility functions
std::string GetResourcePath(std::string resourceName, std::string sceneName);
std::string GetResourcePath(std::string resourceName);
GLuint LoadImageToTexture(std::string resourceName, std::string sceneName, int& imageWidth, int& imageHeight);
GLuint LoadImageToTexture(std::string resourceName, std::string sceneName);
GLuint LoadImageToTexture(std::string resourceName, int& imageWidth, int& imageHeight);
GLuint LoadImageToTexture(std::string resourceName);
GfxProgram LoadGraphicsProgram(std::string vertShaderName, std::string fragShaderName, std::string sceneName);
GfxProgram LoadGraphicsProgram(std::string vertShaderName, std::string fragShaderName);

#endif
