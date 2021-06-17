#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "SpriteRender.h"
#include "Shader.h"

class PostProcessor
{
public:
	Shader PostProcessingShader;
	Texture2D Texture;
	GLuint Width, Height;
	GLboolean Confuse, Chaos, Shake;
public:
	PostProcessor(Shader shader, GLuint width, GLuint height);
	void BeginRender();
	void EndRender();
	void Render(GLfloat time);
private:
	GLuint MSFBO, FBO;
	GLuint RBO;
	GLuint VAO;
	void initRenderData();
};

