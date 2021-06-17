#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>

#include "texture.h"
#include "SpriteRender.h"
#include "GameObject.h"

class BallObject: public GameObject
{
public:
	GLfloat Radius;
	GLboolean Stuck;
public:
	BallObject();
	BallObject(glm::vec2 pos, GLfloat radius, glm::vec2 velocity, Texture2D sprite);
	glm::vec2 Move(GLfloat dt, GLuint window_width);
	void Reset(glm::vec2 position, glm::vec2 velocity);
};

