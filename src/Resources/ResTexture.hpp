/* 
 * File:   ResTexture.hpp
 * Author: ivan
 *
 * Created on August 27, 2010, 8:42 PM
 */

#pragma once


#include <GLES2/gl2.h>

#include <tride/Vector3.hpp>

#include "../MyMaemoFW/GLTexture.hpp"

#include "../ResMan/ResMan.hpp"



class ResTexture : public resman::Resource{
	friend class resman::ResMan;

	GLTexture tex;

	tride::Vec2f dim;//texture dimensions

	ResTexture(const Image& image) :
			tex(image),
			dim(image.Width(), image.Height())
	{}
public:
	~ResTexture()throw(){}

	inline tride::Vec2f Dim()const{
		return this->dim;
	}

	inline void Bind()const{
		this->tex.Bind();
	}

private:
	static ting::Ref<ResTexture> Load(pugi::xml_node el, ting::fs::File &fi);
};

