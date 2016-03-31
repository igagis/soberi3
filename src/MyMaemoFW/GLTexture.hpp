/* 
 * File:   GLTexture.hpp
 * Author: ivan
 *
 * Created on September 17, 2010, 3:11 PM
 */

#pragma once

#include <GLES2/gl2.h>

#include <ting/debug.hpp>

#include <tride/Vector3.hpp>

#include "Image.hpp"



class GLTexture {
	//no copying
	GLTexture(const GLTexture& tex);

	//no assigning
	GLTexture& operator=(const GLTexture& tex);

	GLuint tex;

	tride::Vec2f dim;

public:
	GLTexture(const Image& image, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR){
		this->Init(image, minFilter, magFilter);
	}

	GLTexture() :
			tex(0)
	{}

	~GLTexture(){
		this->Destroy();
	}

	void Init(const Image& image, GLint minFilter = GL_LINEAR, GLint magFilter = GL_LINEAR);

	inline void Bind()const{
		ASSERT(glGetError() == GL_NO_ERROR)
		glBindTexture(GL_TEXTURE_2D, this->tex);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline tride::Vec2f Dim()const{
		return this->dim;
	}

private:
	void Destroy(){
		glDeleteTextures(1, &this->tex);
	}
};

