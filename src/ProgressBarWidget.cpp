#include "ProgressBarWidget.hpp"



using namespace ting;



ting::Ref<ProgressBarWidget> ProgressBarWidget::New(const std::string& sprCompl, const std::string& sprUncompl){
	ting::Ref<ResSprite> compl = resman::ResMan::Inst().Load<ResSprite>(sprCompl);
	ting::Ref<ResSprite> uncompl = resman::ResMan::Inst().Load<ResSprite>(sprUncompl);
	return ting::Ref<ProgressBarWidget>(new ProgressBarWidget(compl, uncompl));
}



ProgressBarWidget::ProgressBarWidget(const ting::Ref<ResSprite> completedSpr, const ting::Ref<ResSprite> uncompletedSpr){
	ASSERT(completedSpr)
	ASSERT(uncompletedSpr)
	ASSERT(completedSpr->Dim() == uncompletedSpr->Dim())

	this->completed = ASS(completedSpr)->GetSprite();
	this->uncompleted = ASS(uncompletedSpr)->GetSprite();

	ASSERT(this->completed.tex == completedSpr->GetSprite().tex)
	ASSERT(this->uncompleted.tex == uncompletedSpr->GetSprite().tex)

	this->quadLength = completed.verts[3].x - completed.verts[0].x;
	this->texQuadLength = completed.texCoords[3].x - completed.texCoords[0].x;

	this->Resize(completedSpr->Dim());

	this->SetCompleteness(0);
}



//override
void ProgressBarWidget::Render(const tride::Matr4f& matrix)const{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->completed.Render(matrix);
	this->uncompleted.Render(matrix);
//	TRACE(<< "ProgressBarWidget::Render(): Pos() = " << this->Pos() << " Dim() = " << this->Dim() << std::endl)
}



void ProgressBarWidget::SetCompleteness(float t){
	ASSERT(0 <= t && t <= 1)

	this->completeness = t;

	{
		float dx = this->quadLength * t;
		this->completed.verts[2].x = this->completed.verts[1].x + dx;
		this->completed.verts[3].x = this->completed.verts[0].x + dx;

		float dtx = this->texQuadLength * t;
		this->completed.texCoords[2].x = this->completed.texCoords[1].x + dtx;
		this->completed.texCoords[3].x = this->completed.texCoords[0].x + dtx;
	}

	{
		float dx = this->quadLength * (1 - t);
		this->uncompleted.verts[1].x = this->uncompleted.verts[2].x - dx;
		this->uncompleted.verts[0].x = this->uncompleted.verts[3].x - dx;

		float dtx = this->texQuadLength * (1 - t);
		this->uncompleted.texCoords[1].x = this->uncompleted.texCoords[2].x - dtx;
		this->uncompleted.texCoords[0].x = this->uncompleted.texCoords[3].x - dtx;
	}
}
