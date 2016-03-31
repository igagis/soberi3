#include <sstream>

#include <ting/timer.hpp>

#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/GLWindow.hpp"

#include "ResMan/ResMan.hpp"

#include "SelectLevelWindow.hpp"

#include "MainMenuWindow.hpp"
#include "GameWindow.hpp"



using namespace ting;
using namespace ting::fs;
using namespace tride;



SelectLevelWindow::SelectLevelWindow(ting::Ptr<File> fi) :
		fi(ASS(fi))
{
	ASS(this->backButton)->clicked.Connect(WeakRef<SelectLevelWindow>(this), &SelectLevelWindow::OnBackPressed);

	this->levelList = ListWidget::New(4, 120);
	this->Add(this->levelList);
	this->levelList->itemClicked.Connect(this, &SelectLevelWindow::OnLevelSelected);

	Array<std::string> files = this->fi->ListDirContents();
	TRACE(
			<< "files list = " << std::endl
			<< files
		)

	for(unsigned i = 0, j = 1; i < files.Size(); ++i){
		ASSERT(files[i].size() > 0)
		if(files[i][files[i].size() - 1] == '/')
			continue;

		this->levelList->Add(LevelWidget::New(j));
		++j;
	}
}



void SelectLevelWindow::OnBackPressed(){
	GLWindow::Inst().SetWidget(MainMenuWindow::New());
}



void SelectLevelWindow::OnLevelSelected(ting::Ref<Widget> w){
	ASSERT_INFO(this->fi, "SelectLevelWindow::OnLevelSelected(): file interface is null, create new SelectLevelWindow instance")

	ASSERT(w)

	ting::Ref<GameWindow> gw = GameWindow::New(this->fi);

	ASSERT(w.DynamicCast<LevelWidget>())
	gw->LoadLevel(w.StaticCast<LevelWidget>()->LevelNum());

	GLWindow::Inst().SetWidget(gw);
}



void SelectLevelWindow::OnResize(){
//	TRACE(<< "SelectLevelWindow::OnResize(): invoked" << std::endl)
	this->BasicWindow::OnResize();

//	TRACE(<< "SelectLevelWindow::OnResize(): resizing level list widget" << std::endl)
	ASS(this->levelList)->Resize(tride::Vec2f(
			this->Dim().x,
			this->Dim().y - ASS(this->minimizeButton)->Dim().y
		));

	//This is for testing something...
//	ASS(this->levelList)->Move(Vec2f(
//			30,
//			20
//		));
}



SelectLevelWindow::LevelWidget::LevelWidget(unsigned levelNum) :
		levelNum(levelNum)
{
	this->bg = resman::ResMan::Inst().Load<ResSprite>("spr_level_widget");

	this->Resize(this->bg->Dim());

	this->font = resman::ResMan::Inst().Load<ResFont>("fnt_main");

	std::stringstream ss;
	ss << this->levelNum;
	this->numStr = ss.str();
}



//override
void SelectLevelWindow::LevelWidget::Render(const tride::Matr4f& matrix)const{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->bg->Render(matrix);

	TexFont::BoundingBox bb = this->font->StringBoundingBox(this->numStr);

	Matr4f matr(matrix);
	matr.Translate((this->Dim().x - bb.Width()) / 2 + bb.left, (this->Dim().y - bb.Height()) / 2);

	this->font->RenderString(matr, this->numStr, Vec4f(1, 0, 0, 1));

//	SimpleColoringShader &s = SimpleColoringShader::Inst();
//	s.UseProgram();
//
//	Matr4f matr(matrix);
//	matr.Scale(this->Dim());
//
//	s.SetMatrix(matr);
//
//	s.DisableColorPointer();
//
//	s.SetColor(Vec3f(1, 1, 1));
//
//	s.DrawQuad01(GL_LINE_LOOP);
}


