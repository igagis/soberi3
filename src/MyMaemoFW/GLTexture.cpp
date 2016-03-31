/* 
 * File:   GLTexture.cpp
 * Author: ivan
 * 
 * Created on September 17, 2010, 3:11 PM
 */

#include "GLTexture.hpp"



using namespace ting;
using namespace tride;



void GLTexture::Init(const Image& image, GLint minFilter, GLint magFilter){
	if(this->tex != 0)
		this->Destroy();

	//TODO: assert the image dimensions are power of two

	this->dim = Vec2f(image.Width(), image.Height());

	glGenTextures(1, &this->tex);
	ASSERT(glGetError() == GL_NO_ERROR)
	ASSERT(this->tex != GL_INVALID_VALUE)
	ASSERT(this->tex != 0)
	glActiveTexture(GL_TEXTURE0);
	ASSERT(glGetError() == GL_NO_ERROR)
	glBindTexture(GL_TEXTURE_2D, this->tex);
	ASSERT(glGetError() == GL_NO_ERROR)

	GLint internalFormat;
	switch(image.NumChannels()){
		case 1:
			internalFormat = GL_LUMINANCE;
			break;
		case 2:
			internalFormat = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			internalFormat = GL_RGB;
			break;
		case 4:
			internalFormat = GL_RGBA;
			break;
		default:
			ASSERT(false)
			break;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	ASSERT(glGetError() == GL_NO_ERROR)

	glTexImage2D(
			GL_TEXTURE_2D,
			0,//0th level, no mipmaps
			internalFormat, //internal format
			image.Width(),
			image.Height(),
			0,//border, should be 0!
			internalFormat, //format of the texel data
			GL_UNSIGNED_BYTE,
			image.Buf().Begin()
		);
	ASSERT(glGetError() == GL_NO_ERROR)

	//NOTE: on OpenGL ES 2 it is necessary to set the filter parameters
	//      for every texture!!! Otherwise it may not work!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	ASSERT(glGetError() == GL_NO_ERROR)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	ASSERT(glGetError() == GL_NO_ERROR)
}
