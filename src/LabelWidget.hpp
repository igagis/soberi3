/* 
 * File:   LabelWidget.hpp
 * Author: ivan
 *
 * Created on September 24, 2010, 1:12 PM
 */

#pragma once

#include <string>

#include "MyMaemoFW/Widget.hpp"

#include "Resources/ResFont.hpp"
#include "Resources/ResSprite.hpp"



class LabelWidget : public Widget{
	ting::Ref<const ResFont> font;

	std::string text;

	tride::Vec2f strPos;

	tride::Vec4f color;

	float scale;

protected:
	LabelWidget(const ting::Ref<const ResFont>& font, const std::string& text, tride::Vec4f color = tride::Vec4f(1), float scale = 1);
	
public:
	~LabelWidget()throw(){}
	
	inline static ting::Ref<LabelWidget> New(
			ting::Ref<ResFont> font,
			const std::string& text = std::string(),
			tride::Vec4f color = tride::Vec4f(1),
			float scale = 1
		)
	{
		return ting::Ref<LabelWidget>(
				new LabelWidget(font, text, color, scale)
			);
	}

public:
	void SetText(const std::string& text);

	inline void SetColor(tride::Vec4f color){
		this->color = color;
	}

private:

	void UpdateSize();

	//override
	void Render(const tride::Matr4f& matrix)const;
};



class ImageWidget : public Widget{
	ting::Ref<const ResSprite> sprite;

	float scale;

protected:
	ImageWidget(const ting::Ref<const ResSprite>& sprite, float scale = 1) :
			sprite(ASS(sprite)),
			scale(scale)
	{
		this->Resize(this->sprite->Dim());
	}

public:
	inline static ting::Ref<ImageWidget> New(
			const ting::Ref<const ResSprite>& sprite,
			float scale = 1
		)
	{
		return ting::Ref<ImageWidget>(
				new ImageWidget(sprite, scale)
			);
	}

public:
	
	//override
	void Render(const tride::Matr4f& matrix)const{
		tride::Matr4f matr(matrix);
		matr.Translate(this->sprite->Pivot());

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		this->sprite->Render(matr);
	}
};

