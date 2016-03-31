#pragma once

#include <ting/types.hpp>

#include "MyMaemoFW/TexFont.hpp"
#include "MyMaemoFW/Widget.hpp"
#include "MyMaemoFW/Updater.hpp"

#include "Resources/ResFont.hpp"
#include "Resources/ResSprite.hpp"

#include "BasicWindow.hpp"
#include "LabelWidget.hpp"
#include "Item.hpp"
#include "AnimMan.hpp"



class MainMenuWindow : public MenuWindow, public Updateable{
	ting::Ref<LabelWidget> titleLabel;

	ting::Ref<Button> startActionModeButton;
	ting::Ref<Button> startPuzzleModeButton;
	ting::Ref<Button> showStatsButton;

	ting::Ref<ToggleButton> soundOnOffButton;
	ting::Ref<ToggleButton> musicOnOffButton;

	ting::StaticBuffer<ting::Ref<const ResSprite>, PopItem::NUM_ITEM_TYPES> itemSprites;
	ting::Inited<float, 0> itemBirthTime;
	AnimMan itemsAnimMan;
public:

	MainMenuWindow();

	~MainMenuWindow()throw();

	static inline ting::Ref<MainMenuWindow> New(){
		return ting::Ref<MainMenuWindow>(new MainMenuWindow());
	}

	//override
	void OnResize();

	//override
	bool Update(ting::u32 deltaTimeMillis);

	//override
	void Render(const tride::Matr4f& matrix)const;

	inline void Pause(){
		this->StopUpdating();
	}

	inline void Unpause(){
		this->StartUpdating();
	}

private:
	void OnStartActionModeGame();
	void OnStartPuzzleModeGame();
	void OnShowStats();
};
