// sdl
#include "SDL_image.h"

// glew
#include "gl/glew.h"

#include <string>

#include "TextureCubeArray.h"

REArray<TextureCubeArray*> TextureCubeArray::gContainer;

void TextureCubeArray::AllocateForFrameBuffer(int width, int height, int count, GLint internalFormat, GLenum format, GLenum type, bool bLinearFilter)
{
	this->width = width;
	this->height = height;
	this->count = count;
	this->internalFormat = internalFormat;
	this->format = format;
	this->type = type;

	if (textureID == GL_INVALID_VALUE)
		glGenTextures(1, &textureID);
	glBindTexture(textureType, textureID);
	if (width > 0 && height > 0 && count > 0)
	{
		glTexImage3D(textureType, 0, internalFormat, width, height, count * 6, 0, format, type, NULL);
	}
	glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(textureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, bLinearFilter ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, bLinearFilter ? GL_LINEAR : GL_NEAREST);	
}

void TextureCubeArray::Reallocate(int width, int height, int count)
{
	if (textureID)
	{
		this->width = width;
		this->height = height;
		this->count = count;
		glBindTexture(textureType, textureID);
		glTexImage3D(textureType, 0, internalFormat, width, height, count * 6, 0, format, type, NULL);
	}
}