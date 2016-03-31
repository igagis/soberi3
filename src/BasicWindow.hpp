#pragma once


#include <string>

#include "MyMaemoFW/Widget.hpp"

#include "Resources/ResSprite.hpp"



class ImageButton : public Button{
	ting::Ref<ResSprite> image;

protected:
	ImageButton(const ting::Ref<ResSprite>& image);
public:

	//override
	void Render(const tride::Matr4f& matrix)const;

	static ting::Ref<ImageButton> New(const std::string& imageResourceName);
};



class ImageToggleButton : public ToggleButton{
	ting::Ref<ResSprite> pressedImage;
	ting::Ref<ResSprite> unpressedImage;

	//override
	void Render(const tride::Matr4f& matrix)const;

protected:
	ImageToggleButton(
			const ting::Ref<ResSprite>& pressedImage,
			const ting::Ref<ResSprite>& unpressedImage = ting::Ref<ResSprite>()
		);
public:

	static ting::Ref<ImageToggleButton> New(
			const std::string& pressedImageResourceName,
			const std::string& unpressedImageResourceName = std::string()
		);
};



class BasicWindow : public Widget{
protected:
	ting::Ref<Button> minimizeButton;

	ting::Ref<Button> backButton;

	BasicWindow(const std::string& backButtonSprite);

	//override
	void OnResize();

public:
};



class MenuWindow : public BasicWindow{
	ting::Ref<ResSprite> bg;
	
protected:
	MenuWindow(const std::string& backButtonSprite = std::string("spr_back_button"));

	//override
	void Render(const tride::Matr4f& matrix)const;
public:
};

