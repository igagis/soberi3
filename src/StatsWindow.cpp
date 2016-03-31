/* 
 * File:   StatsWindow.cpp
 * Author: ivan
 * 
 * Created on September 19, 2010, 5:49 PM
 */

#include "MyMaemoFW/GLWindow.hpp"

#include "StatsWindow.hpp"
#include "MainMenuWindow.hpp"
#include "Preferences.hpp"



using namespace ting;
using namespace tride;



StatsWindow::StatsWindow(){
	this->backButton->clicked.Connect(this, &StatsWindow::OnBackPressed);

	Ref<ResFont> font = resman::ResMan::Inst().Load<ResFont>("fnt_main");
	ASSERT(font)

	this->titleLabel = LabelWidget::New(font, "Statistics", Vec4f(1), 2);
	this->Add(this->titleLabel);

	const Vec4f DNumbersColor(0.3, 1, 0.3, 1);

	//create pop items widgets
	for(unsigned i = PopItem::ZERO; i != PopItem::NUM_ITEM_TYPES; ++i){
		ImageAndLabel &il = this->popItemWidgets[ASSCOND(i, < this->popItemWidgets.Size())];
		il.image = ImageWidget::New(PopItem::LoadSpriteForItem(PopItem::E_Type(i)));
		this->Add(ASS(il.image));

		std::stringstream ss;
		ss << Preferences::Inst().GetNumPoppedItems(PopItem::E_Type(i));
		il.label = LabelWidget::New(
				font,
				ss.str(),
				DNumbersColor
			);
		this->Add(ASS(il.label));
	}

	//create chainlock widgets
	{
		ImageAndLabel &il = this->popItemWidgets[ASSCOND(PopItem::NUM_ITEM_TYPES, < this->popItemWidgets.Size())];
		il.image = ImageWidget::New(
				resman::ResMan::Inst().Load<ResSprite>("spr_item_lock")
			);
		this->Add(ASS(il.image));

		std::stringstream ss;
		ss << Preferences::Inst().GetNumChainLocks();
		il.label = LabelWidget::New(
				font,
				ss.str(),
				DNumbersColor
			);
		this->Add(ASS(il.label));
	}

	//create ice cube widgets
	{
		ImageAndLabel &il = this->popItemWidgets[ASSCOND(PopItem::NUM_ITEM_TYPES + 1, < this->popItemWidgets.Size())];
		il.image = ImageWidget::New(
				resman::ResMan::Inst().Load<ResSprite>("spr_ice_cube")
			);
		this->Add(ASS(il.image));

		std::stringstream ss;
		ss << Preferences::Inst().GetNumIceCubes();
		il.label = LabelWidget::New(
				font,
				ss.str(),
				DNumbersColor
			);
		this->Add(ASS(il.label));
	}

	//Create lightnings label
	{
		std::stringstream ss;
		ss << "Lightnings: " << Preferences::Inst().GetNumLightnings();

		this->lightningLabel = LabelWidget::New(font, ss.str(), Vec4f(0.9, 0.9, 1, 1), 1);
		this->Add(this->lightningLabel);
	}

	//Create time spent in game labels
	{
		Vec4f color(0.7, 0.7, 0.7, 1);

		this->timeSpentInGameTitleLabel = LabelWidget::New(font, "Time spent playing:", color, 1);
		this->Add(this->timeSpentInGameTitleLabel);

//		TRACE(<< "StatsWindow::StatsWindow(): time spent in game = " << Preferences::Inst().GetTimeSpentInGame() << std::endl)
		const u32 DMinMillis = 60 * 1000;
		const u32 DHourMillis = 60 * DMinMillis;
		const u32 DDayMillis = 24 * DHourMillis;

		u32 days = Preferences::Inst().GetTimeSpentInGame() / DDayMillis;
		u32 hours = (Preferences::Inst().GetTimeSpentInGame() % DDayMillis) / DHourMillis;
		u32 minutes = (Preferences::Inst().GetTimeSpentInGame() % DHourMillis) / DMinMillis;
		u32 seconds = (Preferences::Inst().GetTimeSpentInGame() % DMinMillis) / 1000;


		if(days != 0 || hours != 0){
			std::stringstream ssdh;
			
			if(days != 0){
				ssdh << days << " days, ";
			}

			ssdh << hours << " hours, ";
			
			this->timeSpentInGameDaysHoursLabel = LabelWidget::New(font, ssdh.str(), color, 0.8);
			this->Add(this->timeSpentInGameDaysHoursLabel);
		}

		{
			std::stringstream ssms;

			//if days or hours are not 0
			if(this->timeSpentInGameDaysHoursLabel || minutes != 0){
				ssms << minutes << " minutes, ";
			}

			ssms << seconds << " seconds";

			this->timeSpentInGameMinsSecsLabel = LabelWidget::New(font, ssms.str(), color, 0.8);
			this->Add(this->timeSpentInGameMinsSecsLabel);
		}
	}
}



//override
void StatsWindow::OnResize(){
	this->BasicWindow::OnResize();

	ASSERT(this->titleLabel)
	this->titleLabel->Move(Vec2f(
			(this->Dim().x - this->titleLabel->Dim().x) / 2,
			this->Dim().y - this->titleLabel->Dim().y - 80
		));

	const unsigned DNumItemsPerRow = 3;
	const float DSpaceBetweenItemRows = 50;

	for(unsigned i = 0; i != this->popItemWidgets.Size(); ++i){
		ImageAndLabel &il = this->popItemWidgets[ASSCOND(i, < this->popItemWidgets.Size())];
		ASSERT(il.image)
		ASSERT(il.label)

		Vec2f imagePos(
				this->Dim().x / (DNumItemsPerRow * 2) + (this->Dim().x / DNumItemsPerRow) * float(i % DNumItemsPerRow)
						- il.image->Dim().x / 2,
				(il.image->Dim().y + DSpaceBetweenItemRows) * float(i / DNumItemsPerRow)
			);

		imagePos = Vec2f(
				imagePos.x,
				this->titleLabel->Pos().y - imagePos.y - il.image->Dim().y - DSpaceBetweenItemRows
			);
		il.image->Move(imagePos);

		il.label->Move(il.image->Pos()
				+ Vec2f(
						(il.image->Dim().x - il.label->Dim().x) / 2,
						-il.label->Dim().y
					)
			);
	}

	ASSERT(this->lightningLabel)
	this->lightningLabel->Move(Vec2f(
			(this->Dim().x - this->lightningLabel->Dim().x) / 2,
			this->popItemWidgets[ASSCOND(this->popItemWidgets.Size() - 1, < this->popItemWidgets.Size())].label->Pos().y
					- this->lightningLabel->Dim().y - DSpaceBetweenItemRows
		));

	ASSERT(this->timeSpentInGameTitleLabel)
	this->timeSpentInGameTitleLabel->Move(Vec2f(
			(this->Dim().x - this->timeSpentInGameTitleLabel->Dim().x) / 2,
			this->lightningLabel->Pos().y - this->timeSpentInGameTitleLabel->Dim().y - DSpaceBetweenItemRows
		));

	{
		Ref<Widget> aaa;

		if(this->timeSpentInGameDaysHoursLabel){
			this->timeSpentInGameDaysHoursLabel->Move(Vec2f(
					(this->Dim().x - this->timeSpentInGameDaysHoursLabel->Dim().x) / 2,
					this->timeSpentInGameTitleLabel->Pos().y - this->timeSpentInGameDaysHoursLabel->Dim().y - DSpaceBetweenItemRows / 2
				));

			aaa = this->timeSpentInGameDaysHoursLabel;
		}else{
			aaa = this->timeSpentInGameTitleLabel;
		}

		ASSERT(this->timeSpentInGameMinsSecsLabel)
		this->timeSpentInGameMinsSecsLabel->Move(Vec2f(
				(this->Dim().x - this->timeSpentInGameMinsSecsLabel->Dim().x) / 2,
				aaa->Pos().y - this->timeSpentInGameMinsSecsLabel->Dim().y - DSpaceBetweenItemRows / 2
			));
	}
}



void StatsWindow::OnBackPressed(){
	GLWindow::Inst().SetWidget(MainMenuWindow::New());
}
