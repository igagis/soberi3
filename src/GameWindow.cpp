#include <sstream>

#ifdef M_MAEMO
#include "LibOSSO.hpp"
#endif

#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/GLWindow.hpp"
#include "MyMaemoFW/Application.hpp"

#include "GameWindow.hpp"
#include "MainMenuWindow.hpp"
#include "utils.hpp"
#include "Preferences.hpp"
#include "MusicBox.hpp"



using namespace ting;
using namespace ting::fs;
using namespace tride;



static const float DMargin = 20;



void GameWindow::OnResize(){
	this->BasicWindow::OnResize();

	ASSERT(this->soundOnOffButton)
	this->soundOnOffButton->Move(
			ASS(this->minimizeButton)->Pos()
					+ Vec2f(ASS(this->minimizeButton)->Dim().x, 0)
		);

	ASSERT(this->musicOnOffButton)
	this->musicOnOffButton->Move(
			ASS(this->soundOnOffButton)->Pos()
					+ Vec2f(ASS(this->soundOnOffButton)->Dim().x, 0)
		);

	if(this->gf){
		this->gf->Move(tride::Vec2f(0, 0));
	}

	ASSERT(this->restartButton)
	this->restartButton->Move(Vec2f(
			this->backButton->Pos().x - this->restartButton->Dim().x,
			this->Dim().y - this->restartButton->Dim().y
		));
}



void GameWindow::SetGameField(ting::Ref<GameField> gameField){
	ASSERT(gameField)

	//if there is a gf already set, remove it from childs
	if(this->gf.IsValid()){
		this->Remove(this->gf);
		this->gf.Reset();
	}

	this->gf = gameField;
	this->Add(this->gf);

	this->gf->levelCompleted.Connect(ting::WeakRef<GameWindow>(this), &GameWindow::OnLevelCompleted);
	this->gf->levelFailed.Connect(ting::WeakRef<GameWindow>(this), &GameWindow::OnLevelFailed);

	this->OnResize();
}



void GameWindow::OnLevelCompleted(){
	ASS(this->gf)->Pause();

	Ref<LevelCompletedDialog> dlg = LevelCompletedDialog::New();
	this->ShowModal(dlg);

	dlg->Resize(this->Dim());

	dlg->BackButton()->clicked.Connect(this, &GameWindow::OnBackClicked);

	dlg->ReplayButton()->clicked.Connect(WeakRef<GameField>(this->gf), &GameField::Init);
	dlg->ReplayButton()->clicked.Connect(WeakRef<GameField>(this->gf), &GameField::Unpause);
	dlg->ReplayButton()->clicked.Connect(WeakRef<Widget>(this->minimizeButton), &Widget::Show);
	dlg->ReplayButton()->clicked.Connect(WeakRef<Widget>(dlg), &Widget::RemoveFromParent);

	dlg->nextLevelButton()->clicked.Connect(this, &GameWindow::OnNextLevelClicked);
	dlg->nextLevelButton()->clicked.Connect(WeakRef<Widget>(this->minimizeButton), &Widget::Show);
	dlg->nextLevelButton()->clicked.Connect(WeakRef<Widget>(dlg), &Widget::RemoveFromParent);

	ASS(this->minimizeButton)->Hide();
}



void GameWindow::OnLevelFailed(){
	ASS(this->gf)->Pause();

	Ref<LevelFailedDialog> dlg = LevelFailedDialog::New();
	this->ShowModal(dlg);

	dlg->Resize(this->Dim());

	dlg->BackButton()->clicked.Connect(this, &GameWindow::OnBackClicked);

	dlg->ReplayButton()->clicked.Connect(WeakRef<GameField>(this->gf), &GameField::Init);
	dlg->ReplayButton()->clicked.Connect(WeakRef<GameField>(this->gf), &GameField::Unpause);
	dlg->ReplayButton()->clicked.Connect(WeakRef<Widget>(this->minimizeButton), &Widget::Show);
	dlg->ReplayButton()->clicked.Connect(WeakRef<Widget>(dlg), &Widget::RemoveFromParent);

	ASS(this->minimizeButton)->Hide();
}



void GameWindow::OnNextLevelClicked(){
	try{
		this->LoadLevel(this->curLevel + 1);
	}catch(File::Exc &e){
		this->OnBackClicked();
	}
}



void GameWindow::LoadLevel(unsigned levelNumber){
	std::stringstream ss;
	ss << levelNumber << ".lvl";

	this->fi->SetPath(ss.str());
	ting::Ref<GameField> gf = GameField::New(*this->fi);

//	TRACE(<< "GameWindow::LoadLevel(): level loaded" << std::endl)

	this->SetGameField(gf);

	this->curLevel = levelNumber;

	ASS(this->restartButton)->SetHidden(!this->gf->IsPuzzleLevel());
}



GameWindow::GameWindow(ting::Ptr<File> f) :
		BasicWindow("spr_pause_button"),
		curLevel(0),
		fi(ASS(f))
{
#ifdef M_MAEMO
	LibOSSO::Inst().wentToSleep.Connect(ting::WeakRef<GameWindow>(this), &GameWindow::OnPauseClicked);
#endif
	Application::Inst().deactivate.Connect(ting::WeakRef<GameWindow>(this), &GameWindow::OnPauseClicked);

	this->backButton->clicked.Connect(this, &GameWindow::OnPauseClicked);

	this->restartButton = ImageButton::New("spr_restart_button");
	this->Add(this->restartButton);
	this->restartButton->clicked.Connect(this, &GameWindow::OnRestartClicked);

	this->soundOnOffButton = ImageToggleButton::New("spr_sound_on_button", "spr_sound_off_button");
	this->soundOnOffButton->SetPressed(!Preferences::Inst().IsSoundMuted());
	this->Add(this->soundOnOffButton);
	this->soundOnOffButton->clicked.Connect(&aumiks::Lib::Inst(), &aumiks::Lib::SetUnmuted);
	this->soundOnOffButton->clicked.Connect(&Preferences::Inst(), &Preferences::SetSoundUnmuted);

	this->musicOnOffButton = ImageToggleButton::New("spr_music_on_button", "spr_music_off_button");
	this->musicOnOffButton->SetPressed(!Preferences::Inst().IsMusicMuted());
	this->Add(this->musicOnOffButton);
	this->musicOnOffButton->clicked.Connect(&MusicBox::Inst(), &MusicBox::SetUnmuted);
	this->musicOnOffButton->clicked.Connect(&Preferences::Inst(), &Preferences::SetMusicUnmuted);
}



//override
void GameWindow::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->gf)
	ASSERT(this->gf->cellSpr)
	for(unsigned i = 0; i < 6; ++i){
		tride::Matr4f matr(matrix);
		matr.Translate(ASS(this->gf->cellSpr)->Dim().x * i, this->Dim().y - ASS(this->gf->cellSpr)->Dim().y);

		ASS(this->gf->cellSpr)->RenderModulating(matr, Vec4f(0.5, 0.5, 0.5, 1));
	}
}



void GameWindow::OnPauseClicked(){
	ASSERT(this->gf)
	if(this->gf->IsPaused())
		return;

	Ref<GamePausedDialog> dlg = GamePausedDialog::New();
	this->ShowModal(dlg);

	dlg->Resize(this->Dim());

	dlg->BackButton()->clicked.Connect(this, &GameWindow::OnBackClicked);
	dlg->ResumeButton()->clicked.Connect(this, &GameWindow::OnResumeClicked);

	this->gf->Pause();

	ASS(this->minimizeButton)->Hide();

	GLWindow::Inst().SetDirty();
}



void GameWindow::OnRestartClicked(){
	if(!this->gf)
		return;

	this->gf->Init();
}



void GameWindow::OnResumeClicked(){
	ASSERT(this->gf)
	this->gf->Unpause();

	ASS(this->minimizeButton)->Show();
}



void GameWindow::OnBackClicked(){
	GLWindow::Inst().SetWidget(MainMenuWindow::New());
}



InGameDialog::InGameDialog(){
	this->minimizeButton = ImageButton::New("spr_minimize_button");
	this->Add(this->minimizeButton);
	this->minimizeButton->clicked.Connect(
			static_cast<Application*>(&Application::Inst()),
			&Application::Minimize
		);
}



//override
void InGameDialog::Render(const Matr4f& matrix)const{
	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();

	s.DisableColorPointer();

	s.SetColor(Vec4f(0, 0, 0, 0.5));

	Matr4f matr(matrix);
	matr.Scale(this->Dim());

	s.SetMatrix(matr);
	
	s.DrawQuad01();
}



//override
void InGameDialog::OnResize(){
	ASSERT(this->minimizeButton)
	this->minimizeButton->Move(Vec2f(
			0,
			this->Dim().y - this->minimizeButton->Dim().y
		));
}



GamePausedDialog::GamePausedDialog(){
	this->dialog = Dialog::New();
	this->Add(this->dialog);
	this->dialog->resumeButton->clicked.Connect(static_cast<Widget*>(this), &Widget::RemoveFromParent);
}



//override
void GamePausedDialog::OnResize(){
	this->InGameDialog::OnResize();

	ASSERT(this->dialog)
	this->dialog->Move((this->Dim() - this->dialog->Dim()) / 2);
}



GamePausedDialog::Dialog::Dialog(){
	this->backButton = ImageButton::New("spr_back_button");
	this->Add(this->backButton);
	this->backButton->Move(Vec2f(DMargin));

	this->resumeButton = ImageButton::New("spr_resume_button");
	this->Add(this->resumeButton);
	this->resumeButton->Move(Vec2f(this->backButton->TopRightPos().x + DMargin, DMargin));

	this->Resize(Vec2f(this->resumeButton->TopRightPos().x + DMargin, 150));

	this->titleLabel = LabelWidget::New(resman::ResMan::Inst().Load<ResFont>("fnt_main"), "Paused");
	this->titleLabel->SetColor(Vec4f(1, 1, 0, 1));
	this->Add(this->titleLabel);
	this->titleLabel->Move(Vec2f(
			(this->Dim().x - this->titleLabel->Dim().x) / 2,
			this->backButton->TopRightPos().y
					+ (this->Dim().y - (this->backButton->TopRightPos().y) - this->titleLabel->Dim().y) / 2
		));
}



LevelCompletedDialog::LevelCompletedDialog(){
	this->dialog = Dialog::New();
	this->Add(this->dialog);
}



//override
void LevelCompletedDialog::OnResize(){
	this->InGameDialog::OnResize();

	ASSERT(this->dialog)
	this->dialog->Move((this->Dim() - this->dialog->Dim()) / 2);
}



LevelCompletedDialog::Dialog::Dialog(){
	this->backButton = ImageButton::New("spr_back_button");
	this->Add(this->backButton);
	this->backButton->Move(Vec2f(DMargin));

	this->restartButton = ImageButton::New("spr_restart_button");
	this->Add(this->restartButton);
	this->restartButton->Move(Vec2f(this->backButton->TopRightPos().x + DMargin, DMargin));

	this->nextLevelButton = ImageButton::New("spr_next_button");
	this->Add(this->nextLevelButton);
	this->nextLevelButton->Move(Vec2f(this->restartButton->TopRightPos().x + DMargin, DMargin));

	this->Resize(Vec2f(this->nextLevelButton->TopRightPos().x + DMargin, 150));

	this->titleLabel = LabelWidget::New(resman::ResMan::Inst().Load<ResFont>("fnt_main"), "Level Completed!");
	this->titleLabel->SetColor(Vec4f(0.3, 1, 0.3, 1));
	this->Add(this->titleLabel);
	this->titleLabel->Move(Vec2f(
			(this->Dim().x - this->titleLabel->Dim().x) / 2,
			this->backButton->TopRightPos().y
					+ (this->Dim().y - (this->backButton->TopRightPos().y) - this->titleLabel->Dim().y) / 2
		));
}



//override
void InGameDialog::Dialog::Render(const Matr4f& matrix)const{
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	RenderRoundedSquare(matrix, Vec4f(0, 0, 0, 0.8), this->Dim(), 30);
}



LevelFailedDialog::LevelFailedDialog(){
	this->dialog = Dialog::New();
	this->Add(this->dialog);
}



//override
void LevelFailedDialog::OnResize(){
	this->InGameDialog::OnResize();

	ASSERT(this->dialog)
	this->dialog->Move((this->Dim() - this->dialog->Dim()) / 2);
}



LevelFailedDialog::Dialog::Dialog(){
	this->backButton = ImageButton::New("spr_back_button");
	this->Add(this->backButton);
	this->backButton->Move(Vec2f(DMargin));

	this->restartButton = ImageButton::New("spr_restart_button");
	this->Add(this->restartButton);
	this->restartButton->Move(Vec2f(120, DMargin));

	this->Resize(Vec2f(this->restartButton->TopRightPos().x + DMargin, 150));

	this->titleLabel = LabelWidget::New(resman::ResMan::Inst().Load<ResFont>("fnt_main"), "Level Failed!");
	this->titleLabel->SetColor(Vec4f(1, 0, 0, 1));
	this->Add(this->titleLabel);
	this->titleLabel->Move(Vec2f(
			(this->Dim().x - this->titleLabel->Dim().x) / 2,
			this->backButton->TopRightPos().y
					+ (this->Dim().y - (this->backButton->TopRightPos().y) - this->titleLabel->Dim().y) / 2
		));
}
