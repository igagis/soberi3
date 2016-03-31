/* 
 * File:   Sprite.hpp
 * Author: ivan
 *
 * Created on August 28, 2010, 12:54 AM
 */

#pragma once


#include "Resources/ResTexture.hpp"



class Sprite{
public:
	ting::Ref<ResTexture> tex;

	ting::StaticBuffer<tride::Vec2f, 4> verts;
	ting::StaticBuffer<tride::Vec2f, 4> texCoords;

	void Render(const tride::Matr4f& matrix)const;

	//Render using modulation color
	void RenderModulating(const tride::Matr4f& matrix, const tride::Vec4f& color)const;

	//Render using saturation color
	void RenderSaturating(const tride::Matr4f& matrix, const tride::Vec4f& color)const;

	void Scale(tride::Vec2f scale);

	inline float Left()const{
		//Assume vertices 0 and 1 have the same x coordinate
		return this->verts[0].x;
	}

	inline float Right()const{
		//Assume vertices 2 and 3 have the same x coordinate
		return this->verts[2].x;
	}

	inline float Top()const{
		//Assume vertices 1 and 2 have the same y coordinate
		return this->verts[1].y;
	}

	inline float Bottom()const{
		//Assume vertices 0 and 3 have the same y coordinate
		return this->verts[0].y;
	}
};

