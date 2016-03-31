/* 
 * File:   ResSprite.hpp
 * Author: ivan
 *
 * Created on August 27, 2010, 8:46 PM
 */

#pragma once


#include "ResTexture.hpp"

#include "../Sprite.hpp"



class ResSprite : public resman::Resource{
	friend class resman::ResMan;

	Sprite spr;

	tride::Vec2f dim;
	tride::Vec2f pivot;

	ResSprite(
			const ting::Ref<ResTexture> &texture,
			const tride::Vec2f& xy,
			const tride::Vec2f& wh,
			const tride::Vec2f& dimensions,
			const tride::Vec2f& pivot
		);
public:

	inline const Sprite& GetSprite()const{
		return this->spr;
	}

	inline tride::Vec2f Dim()const{
		return this->dim;
	}

	inline tride::Vec2f Pivot()const{
		return this->pivot;
	}

	inline float Left()const{
		return this->spr.Left();
	}

	inline float Right()const{
		return this->spr.Right();
	}

	inline float Top()const{
		return this->spr.Top();
	}

	inline float Bottom()const{
		return this->spr.Bottom();
	}

	/**
	 * @brief Apply permanent scale to the sprite.
	 * @param scale - scale coefficients in x and y directions.
	 */
	void Scale(tride::Vec2f scale);

	/**
	 * @brief Apply permanent scale to the sprite.
	 * @param scale - scale coefficient.
	 */
	inline void Scale(float scale){
		this->Scale(tride::Vec2f(scale, scale));
	}

	inline void Render(const tride::Matr4f& matrix)const{
		this->spr.Render(matrix);
	}

	//Render using modulation color
	inline void RenderModulating(const tride::Matr4f& matrix, const tride::Vec4f& color)const{
		this->spr.RenderModulating(matrix, color);
	}

	//Render using saturation color
	inline void RenderSaturating(const tride::Matr4f& matrix, const tride::Vec4f& color)const{
		this->spr.RenderSaturating(matrix, color);
	}

private:
	static ting::Ref<ResSprite> Load(pugi::xml_node el, ting::fs::File &fi);
};

