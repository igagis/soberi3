#pragma once

#include <string>

#include "MyMaemoFW/Widget.hpp"
#include "MyMaemoFW/Updater.hpp"

#include "Resources/ResSprite.hpp"
#include "Resources/ResFont.hpp"

#include "BasicWindow.hpp"
#include "ListWidget.hpp"



class SelectLevelWindow : public MenuWindow{
	ting::Ref<ListWidget> levelList;

	ting::Ptr<ting::fs::File> fi;

protected:
	SelectLevelWindow(ting::Ptr<ting::fs::File> fi);

public:
	static inline ting::Ref<SelectLevelWindow> New(ting::Ptr<ting::fs::File> fi){
		return ting::Ref<SelectLevelWindow>(
				new SelectLevelWindow(fi)
			);
	}

private:
	//override
	void OnResize();

	void OnBackPressed();

	void OnLevelSelected(ting::Ref<Widget> w);

	class LevelWidget : public Widget{
		ting::Ref<ResSprite> bg;
		ting::Ref<ResFont> font;

		std::string numStr;

		unsigned levelNum;
	public:
		LevelWidget(unsigned levelNum);
		
		~LevelWidget()throw(){}

		static inline ting::Ref<LevelWidget> New(unsigned levelNum){
			return ting::Ref<LevelWidget>(new LevelWidget(levelNum));
		}

		inline unsigned LevelNum()const{
			return this->levelNum;
		}

		//override
		void Render(const tride::Matr4f& matrix)const;
	};
};
