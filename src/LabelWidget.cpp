/* 
 * File:   LabelWidget.cpp
 * Author: ivan
 * 
 * Created on September 24, 2010, 1:12 PM
 */

#include "LabelWidget.hpp"



using namespace ting;
using namespace tride;



LabelWidget::LabelWidget(const ting::Ref<const ResFont>& font, const std::string& text, tride::Vec4f color, float scale) :
		font(ASS(font)),
		scale(scale)
{
	this->SetText(text);
	this->SetColor(color);
}



//override
void LabelWidget::Render(const tride::Matr4f& matrix)const{
	ModulatingColoringTexturingShader& s = ModulatingColoringTexturingShader::Inst();

	s.DisableColorPointer();
	s.SetColor(this->color);

	Matr4f matr(matrix);
	matr.Scale(this->scale, this->scale);
	matr.Translate(this->strPos);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	this->font->RenderString(s, matr, this->text);

//	{
//		SimpleColoringShader &s = SimpleColoringShader::Inst();
//		s.UseProgram();
//
//		Matr4f matr(matrix);
//		matr.Scale(this->Dim());
//
//		s.SetMatrix(matr);
//
//		s.DisableColorPointer();
//
//		s.SetColor(Vec3f(1, 1, 1));
//
//		s.DrawQuad01(GL_LINE_LOOP);
//	}
}



void LabelWidget::SetText(const std::string& text){
	this->text = text;

	this->UpdateSize();
}



void LabelWidget::UpdateSize(){
	TexFont::BoundingBox bb = ASS(this->font)->StringBoundingBox(text);

	this->strPos.x = bb.left;
	this->strPos.y = bb.bottom;

	this->Resize(Vec2f(bb.Width(), bb.Height()) * this->scale);
}
