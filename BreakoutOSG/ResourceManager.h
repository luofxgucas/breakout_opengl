#pragma once

#include <map>
#include <string>

#include <GL/glew.h>

#include "texture.h"
#include "Shader.h"

using namespace std;

class ResourceManager
{
public:
	static map<string, Shader> Shaders;
	static map<string, Texture2D> Textures;

	static Shader LoadShader(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile, string name);
	static Shader GetShader(std::string name);
	static Texture2D LoadTexture(const GLchar* file, GLboolean alpha, string name);
	static Texture2D GetTexture(string name);
	static void Clear();
private:
	ResourceManager() {}
	static Shader loadShaderFromFile(const GLchar* vShaderFile, const GLchar* fShaderFile, const GLchar* gShaderFile = nullptr);
	static Texture2D loadTextureFromFile(const GLchar* file, GLboolean alpha);
};

