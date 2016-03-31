/* 
 * File:   StatsWindow.hpp
 * Author: ivan
 *
 * Created on September 19, 2010, 5:49 PM
 */

#pragma once

#include <ting/Buffer.hpp>

#include "Resources/ResFont.hpp"
#include "Resources/ResSprite.hpp"

#include "BasicWindow.hpp"
#include "Item.hpp"
#include "LabelWidget.hpp"



class StatsWindow : public MenuWindow{
	ting::Ref<LabelWidget> titleLabel;

	class ImageAndLabel{
	public:
		ting::Ref<ImageWidget> image;
		ting::Ref<LabelWidget> label;
	};

	//NOTE: +2 is for ice cube and chain lock
	ting::StaticBuffer<ImageAndLabel, PopItem::NUM_ITEM_TYPES + 2> popItemWidgets;

	ting::Ref<LabelWidget> lightningLabel;

	ting::Ref<LabelWidget> timeSpentInGameTitleLabel;
	ting::Ref<LabelWidget> timeSpentInGameDaysHoursLabel;
	ting::Ref<LabelWidget> timeSpentInGameMinsSecsLabel;
	
public:
	StatsWindow();

	static inline ting::Ref<StatsWindow> New(){
		return ting::Ref<StatsWindow>(new StatsWindow());
	}

private:
	void OnBackPressed();

	//override
	void OnResize();
};
