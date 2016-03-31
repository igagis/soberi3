/* 
 * File:   ProgressBarWidget.hpp
 * Author: ivan
 *
 * Created on August 29, 2010, 10:45 PM
 */

#pragma once


#include "MyMaemoFW/Widget.hpp"

#include "Resources/ResSprite.hpp"
#include "Sprite.hpp"



class ProgressBarWidget : public Widget{
	Sprite completed;
	Sprite uncompleted;

	float quadLength;
	float texQuadLength;

	float completeness;

	ProgressBarWidget(const ting::Ref<ResSprite> completedSpr, const ting::Ref<ResSprite> uncompletedSpr);
public:

	//override
	void Render(const tride::Matr4f& matrix)const;

	static ting::Ref<ProgressBarWidget> New(const std::string& sprCompl, const std::string& sprUncompl);

	float Completeness()const{
		return this->completeness;
	}

	void SetCompleteness(float t);
};
