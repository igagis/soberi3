#include "BasicWindow.hpp"

#include "MyMaemoFW/GLWindow.hpp"
#include "MyMaemoFW/Application.hpp"

#include "ResMan/ResMan.hpp"



using namespace ting;
using namespace tride;



ting::Ref<ImageButton> ImageButton::New(const std::string& imageResourceName){
	ting::Ref<ResSprite> image = resman::ResMan::Inst().Load<ResSprite>(imageResourceName);
	return ting::Ref<ImageButton>(new ImageButton(image));
}



ImageButton::ImageButton(const ting::Ref<ResSprite>& image) :
		image(ASS(image))
{
	ASSERT(this->image.IsValid())

	this->Resize(this->image->Dim());
}



void ImageButton::Render(const tride::Matr4f& matrix)const{
//	TRACE(<< "ImageButton::Render(): enter" << std::endl)
	Matr4f matr(matrix);

//	TRACE(<< "ImageButton::Render(): matrix = " << matr << std::endl)

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	ASS(this->image)->Render(matr);

	if(this->isPressed){
		SimpleColoringShader &s = SimpleColoringShader::Inst();
		s.UseProgram();

		Matr4f matr(matrix);
		matr.Scale(this->Dim());

		s.SetMatrix(matr);

		s.DisableColorPointer();
		s.SetColor(Vec4f(1, 1, 1, 0.2));

		s.DrawQuad01(GL_TRIANGLE_FAN);
	}
}



ImageToggleButton::ImageToggleButton(
		const ting::Ref<ResSprite>& pressedImage,
		const ting::Ref<ResSprite>& unpressedImage
	) :
		pressedImage(ASS(pressedImage)),
		unpressedImage(unpressedImage)
{
	ASSERT(this->pressedImage)

	//assert that both images have equal dimensions
	ASSERT(
			this->unpressedImage.IsNotValid()
					|| this->pressedImage->Dim() == this->unpressedImage->Dim()
		)

	this->Resize(this->pressedImage->Dim());
}



//static
ting::Ref<ImageToggleButton> ImageToggleButton::New(
		const std::string& pressedImageResourceName,
		const std::string& unpressedImageResourceName
	)
{
	ting::Ref<ResSprite> pressedImage
			= resman::ResMan::Inst().Load<ResSprite>(pressedImageResourceName);

	ting::Ref<ResSprite> unpressedImage;
	if(unpressedImageResourceName.size() > 0){
			unpressedImage = resman::ResMan::Inst().Load<ResSprite>(unpressedImageResourceName);
	}
	
	return ting::Ref<ImageToggleButton>(new ImageToggleButton(
			pressedImage,
			unpressedImage
		));
}



//override
void ImageToggleButton::Render(const tride::Matr4f& matrix)const{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if(this->isPressed){
		ASS(this->pressedImage)->Render(matrix);
	}else{
		//if there is unpressed image, then render it
		if(this->unpressedImage){
			this->unpressedImage->Render(matrix);
		}else{//no unpressed image, just dim the pressed one by half
			ASS(this->pressedImage)->Render(matrix);
			
			SimpleColoringShader &s = SimpleColoringShader::Inst();
			s.UseProgram();

			Matr4f matr(matrix);
			matr.Scale(this->Dim());

			s.SetMatrix(matr);

			s.DisableColorPointer();
			s.SetColor(Vec4f(0, 0, 0, 0.5));

			s.DrawQuad01(GL_TRIANGLE_FAN);
		}
	}
}



BasicWindow::BasicWindow(const std::string& backButtonSprite){
	this->minimizeButton = ImageButton::New("spr_minimize_button");
	this->Add(this->minimizeButton);
	this->minimizeButton->clicked.Connect(
			static_cast<Application*>(&Application::Inst()),
			&Application::Minimize
		);

	this->backButton = ImageButton::New(backButtonSprite);
	this->Add(this->backButton);
}



//override
void BasicWindow::OnResize(){
	ASS(this->minimizeButton)->Move(tride::Vec2f(
			0,
			this->Dim().y - ASS(this->minimizeButton)->Dim().y
		));

	ASS(this->backButton)->Move(Vec2f(
			this->Dim().x - this->backButton->Dim().x,
			this->Dim().y - this->backButton->Dim().y
		));
}



MenuWindow::MenuWindow(const std::string& backButtonSprite) :
		BasicWindow(backButtonSprite)
{
	this->bg = resman::ResMan::Inst().Load<ResSprite>("spr_grass");
}



//override
void MenuWindow::Render(const tride::Matr4f& matrix)const{
	glDisable(GL_BLEND);
	for(unsigned j = 0; j < 10; ++j){
		for(unsigned i = 0; i < 6; ++i){
			Matr4f matr(matrix);
			matr.Translate(this->bg->Dim().CompMul(Vec2f(i, j)));

			this->bg->RenderModulating(matr, Vec4f(0.7, 0.7, 0.7, 1));
//			this->bg->Render(matr);
		}//~for(i)
	}//~for(j)
}
