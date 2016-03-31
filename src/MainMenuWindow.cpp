#include <string>
#include <cmath>

#include <aumiks/aumiks.hpp>

#include "MyMaemoFW/Application.hpp"
#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/Application.hpp"
#include "MyMaemoFW/GLWindow.hpp"

#include "AnimMan.hpp"
#include "MainMenuWindow.hpp"
#include "SelectLevelWindow.hpp"
#include "GameWindow.hpp"
#include "StatsWindow.hpp"
#include "LabelWidget.hpp"
#include "Item.hpp"
#include "Random.hpp"
#include "utils.hpp"
#include "Preferences.hpp"
#include "MusicBox.hpp"

#ifdef M_MAEMO
#include "LibOSSO.hpp"
#endif


using namespace ting;
using namespace ting::fs;
using namespace tride;



namespace{
class MainMenuButton : public Button{
	ting::Ref<LabelWidget> label;
	
	ting::Ref<ResSprite> spr;

protected:
	MainMenuButton(const std::string& text, PopItem::E_Type popItemType){
		this->label = LabelWidget::New(
				resman::ResMan::Inst().Load<ResFont>("fnt_main"),
				text,
				Vec4f(1),
				1
			);
		this->Add(this->label);

		this->spr = PopItem::LoadSpriteForItem(popItemType);

		this->Resize(Vec2f(
				this->label->Dim().x + this->spr->Dim().x * 2,
				std::max(this->spr->Dim().y, this->label->Dim().y)
			));

		this->label->Move((this->Dim() - this->label->Dim()) / 2);
	}

public:
	inline static ting::Ref<MainMenuButton> New(const std::string& text, PopItem::E_Type popItemType){
		return ting::Ref<MainMenuButton>(new MainMenuButton(text, popItemType));
	}

	//override
	void Render(const tride::Matr4f& matrix)const{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		{
			Vec4f color;
			if(this->isPressed){
				color = Vec4f(1, 1, 1, 0.3);
			}else{
				color = Vec4f(0, 0, 0, 0.7);
			}

			RenderRoundedSquare(matrix, color, this->Dim(), 20);
		}

		{
			Matr4f matr(matrix);
			matr.Translate(this->spr->Pivot());
			this->spr->Render(matr);
		}

		{
			Matr4f matr(matrix);
			matr.Translate(this->Dim() - this->spr->Dim() + this->spr->Pivot());
			this->spr->Render(matr);
		}
	}
};
}//~namespace



MainMenuWindow::MainMenuWindow() :
		MenuWindow("spr_exit_button")
{
//	TRACE(<< "MainMenuWindow::MainMenuWindow(): enter" << std::endl)

#ifdef M_MAEMO
	LibOSSO::Inst().wentToSleep.Connect(
			ting::WeakRef<MainMenuWindow>(this),
			&MainMenuWindow::Pause
		);
#endif

	Application::Inst().deactivate.Connect(
			ting::WeakRef<MainMenuWindow>(this),
			&MainMenuWindow::Pause
		);
	Application::Inst().activate.Connect(
			ting::WeakRef<MainMenuWindow>(this),
			&MainMenuWindow::Unpause
		);

	this->titleLabel = LabelWidget::New(resman::ResMan::Inst().Load<ResFont>("fnt_main"), "Forest Smash", Vec4f(1, 1, 0.5, 1), 2);
	this->Add(this->titleLabel);

	ASS(this->backButton)->clicked.Connect(&Application::Inst(), &Application::Quit);

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

	this->startActionModeButton = MainMenuButton::New("Play Action", PopItem::ZERO);
	this->Add(this->startActionModeButton);
	this->startActionModeButton->clicked.Connect(this, &MainMenuWindow::OnStartActionModeGame);

	this->startPuzzleModeButton = MainMenuButton::New("Play Puzzle", PopItem::ONE);
	this->Add(this->startPuzzleModeButton);
	this->startPuzzleModeButton->clicked.Connect(this, &MainMenuWindow::OnStartPuzzleModeGame);

	this->showStatsButton = MainMenuButton::New("Statistics", PopItem::TWO);
	this->Add(this->showStatsButton);
	this->showStatsButton->clicked.Connect(this, &MainMenuWindow::OnShowStats);

	//Load item sprites:
	for(unsigned i = 0; i < PopItem::NUM_ITEM_TYPES; ++i){
		this->itemSprites[i] = PopItem::LoadSpriteForItem(PopItem::E_Type(i));
	}

	this->StartUpdating();
}



MainMenuWindow::~MainMenuWindow()throw(){
//	TRACE(<< "MainMenuWindow::~MainMenuWindow(): invoked" << std::endl)
}



void MainMenuWindow::OnResize(){
//	TRACE(<< "MainMenuWindow::OnResize()" << std::endl)
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

	ASSERT(this->titleLabel)
	this->titleLabel->Move(Vec2f(
			(this->Dim().x - this->titleLabel->Dim().x) / 2,
			this->Dim().y - 150
		));

	const float DSpacing = 50;

	ASSERT(this->startActionModeButton)
	this->startActionModeButton->Move(Vec2f(
			floor((this->Dim().x - this->startActionModeButton->Dim().x) / 2),
			this->titleLabel->Pos().y - this->startActionModeButton->Dim().y - 100
		));

	ASSERT(this->startPuzzleModeButton)
	this->startPuzzleModeButton->Move(Vec2f(
			floor((this->Dim().x - this->startPuzzleModeButton->Dim().x) / 2),
			this->startActionModeButton->Pos().y - this->startPuzzleModeButton->Dim().y - DSpacing
		));

	ASSERT(this->showStatsButton)
	this->showStatsButton->Move(Vec2f(
			floor((this->Dim().x - this->showStatsButton->Dim().x) / 2),
			this->startPuzzleModeButton->Pos().y - this->showStatsButton->Dim().y - DSpacing
		));
}



void MainMenuWindow::OnStartActionModeGame(){
	TRACE(<< "MainMenuWindow::OnStartActionModeGame(): invoked" << std::endl)

#ifdef M_ZIP_RES
	ting::Ptr<ZipFile> fi(new ZipFile("lvl_action.zip"));
	fi->OpenZipFile();
#else
	ting::Ptr<FSFile> fi(new FSFile());
	fi->SetRootDir("lvl/action/");
#endif

	GLWindow::Inst().SetWidget(SelectLevelWindow::New(fi));
}



void MainMenuWindow::OnStartPuzzleModeGame(){
	TRACE(<< "MainMenuWindow::OnStartPuzzleModeGame(): invoked" << std::endl)

#ifdef M_ZIP_RES
	ting::Ptr<ZipFile> fi(new ZipFile("lvl_puzzle.zip"));
	fi->OpenZipFile();
#else
	ting::Ptr<FSFile> fi(new FSFile());
	fi->SetRootDir("lvl/puzzle/");
#endif

	GLWindow::Inst().SetWidget(SelectLevelWindow::New(fi));
}



void MainMenuWindow::OnShowStats(){
	GLWindow::Inst().SetWidget(StatsWindow::New());
}



namespace{
class ItemAnimation : public Animation{
	const ting::Ref<const ResSprite> sprite;

	float scale;
	float w;
public:
	ItemAnimation(const ting::Ref<const ResSprite>& sprite) :
			sprite(sprite)
	{
		this->totalTime = 3;

		this->scale = 0.5 + 0.5 * GlobalRandom::Inst().Rnd().RandFloatZeroOne();
		this->w = (int(ASSCOND(GlobalRandom::Inst().Rnd().Rand(2), < 2) * 2) - 1);//-1 or 1
		ASSERT_INFO(ting::math::Abs(this->w) == 1, "this->w = " << this->w)
	}

	//override
	void Render(const tride::Matr4f& matrix)const{
		ASSERT(this->time <= this->totalTime)
		ASSERT(this->time >= 0)
		ASSERT(this->totalTime != 0)
		float t = this->time / this->totalTime;

		Matr4f matr(matrix);
		matr.Translate(this->p);
		matr.Rotate(this->w * t * 3);

		const float DThr = 0.2;
		if(t < DThr){
			matr.Scale(this->scale * (t / DThr));
		}else if(t < (1 - DThr)){
			matr.Scale(this->scale);
		}else{
			ASSERT(t >= (1 - DThr))
			matr.Scale(this->scale * (1 - (t - 1 + DThr) / DThr));
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		ASS(this->sprite)->RenderModulating(matr, Vec4f(0.7, 0.7, 0.7, 1));
	}
};
}



//override
void MainMenuWindow::Render(const tride::Matr4f& matrix)const{
	this->MenuWindow::Render(matrix);

	this->itemsAnimMan.Render(matrix);
}



//override
bool MainMenuWindow::Update(ting::u32 deltaTimeMillis){
	float dt = float(deltaTimeMillis) / 1000.0f;

//	TRACE(<< "MainMenuWindow::Update(): invoked, deltaTimeMillis = " << deltaTimeMillis << " dt = " << dt << std::endl)

	this->itemsAnimMan.Update(dt);

	const float DBirthTimeStep = 0.1;
	const float DProbabilityPerSecond = 0.9;

	this->itemBirthTime += dt;

	//clamp top to prevent enormous number of items generated in case of a long time lag
	if(this->itemBirthTime > DBirthTimeStep)
		this->itemBirthTime = DBirthTimeStep;

	while(this->itemBirthTime > 0){
		this->itemBirthTime -= DBirthTimeStep;
		for(unsigned i = 0; i < 10; ++i){
			if(
					GlobalRandom::Inst().Rnd().RandFloatZeroOne()
							< (1 - ting::math::Exp(DBirthTimeStep * ting::math::Ln(1 - DProbabilityPerSecond)))
				)
			{
				ting::Ptr<Animation> a(new ItemAnimation(
						this->itemSprites[
								GlobalRandom::Inst().Rnd().Rand(PopItem::NUM_ITEM_TYPES)
							]
					));

				a->p = Vec2f(
						GlobalRandom::Inst().Rnd().Rand(this->Dim().x),
						GlobalRandom::Inst().Rnd().Rand(this->Dim().y)
					);

	//			TRACE(<< "MainMenuWindow::Update(): anim added, p = " << a->p << std::endl)

				this->itemsAnimMan.AddAnim(a);
			}
		}
	}


	GLWindow::Inst().SetDirty();
	return false;
}
