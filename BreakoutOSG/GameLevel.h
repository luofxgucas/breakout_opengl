#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GameObject.h"
#include "SpriteRender.h"
#include "ResourceManager.h"

using namespace std;

class GameLevel
{
public:
	vector<GameObject> Bricks;
	GameLevel() {}
	void Load(const GLchar* file, GLuint levelWidth, GLuint levelHeight);
	void Draw(SpriteRender& renderer);
	GLboolean IsCompleted();
private:
	void init(vector<vector<GLuint>> tileData, GLuint levelWidth, GLuint levelHeight);
};

