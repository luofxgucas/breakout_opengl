#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.h"
#include "Shader.h"

class SpriteRender
{
public:
	SpriteRender(Shader& shader);
	~SpriteRender();
	void DrawSprite(Texture2D& texture, glm::vec2 position, glm::vec2 size = glm::vec2(10, 10),
		GLfloat rotate = 0.0f, glm::vec3 color = glm::vec3(1.0f));
private:
	Shader shader;
	GLuint quadVAO;
	void initRenderData();
};

