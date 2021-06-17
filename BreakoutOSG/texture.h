#pragma once

#include <GL/glew.h>

class Texture2D
{
public:
	Texture2D();
public:
	GLuint ID;
	GLuint Width, Height;
	GLuint Internal_Format;
	GLuint Image_Format;
	GLuint Wrap_S;
	GLuint Wrap_T;
	GLuint Filter_Min;
	GLuint Filter_Max;
	
	void Generate(GLuint width, GLuint height, unsigned char* data);
	void Bind() const;
};

