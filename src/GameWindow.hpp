#pragma once

#include <ting/fs/File.hpp>

#include "Resources/ResSprite.hpp"

#include "BasicWindow.hpp"
#include "GameField.hpp"
#include "LabelWidget.hpp"



class GameWindow : public BasicWindow{
	unsigned curLevel;

	ting::Ref<GameField> gf;

	ting::Ptr<ting::fs::File> fi;

	ting::Ref<Button> restartButton;

	ting::Ref<ToggleButton> soundOnOffButton;
	ting::Ref<ToggleButton> musicOnOffButton;
public:
	GameWindow(ting::Ptr<ting::fs::File> f);

	static inline ting::Ref<GameWindow> New(ting::Ptr<ting::fs::File> f){
		return ting::Ref<GameWindow>(new GameWindow(f));
	}

	void SetGameField(ting::Ref<GameField> gameField);

	//override
	void OnResize();

	//override
	void Render(const tride::Matr4f& matrix)const;

	void LoadLevel(unsigned levelNumber);

private:
	void OnLevelCompleted();

	void OnLevelFailed();

	void OnPauseClicked();

	void OnRestartClicked();

	void OnResumeClicked();

	void OnBackClicked();

	void OnNextLevelClicked();
};



class InGameDialog : public Widget{
	ting::Ref<Button> minimizeButton;

	//override
	void Render(const tride::Matr4f& matrix)const;

protected:
	//override
	void OnResize();

public:

	InGameDialog();


private:
	//override
	bool OnMouseClick(const tride::Vec2f& pos, EMouseButton button, bool isDown){
		return true;
	}
public:

	class Dialog : public Widget{
	public:

		//override
		void Render(const tride::Matr4f& matrix)const;
	};
};



class GamePausedDialog : public InGameDialog{
	class Dialog : public InGameDialog::Dialog{
	public:
		ting::Ref<Button> resumeButton;
		ting::Ref<Button> backButton;
		ting::Ref<LabelWidget> titleLabel;

		Dialog();

		static inline ting::Ref<Dialog> New(){
			return ting::Ref<Dialog>(new Dialog());
		}
	};

	ting::Ref<Dialog> dialog;

	//override
	void OnResize();
	
public:

	GamePausedDialog();

	static inline ting::Ref<GamePausedDialog> New(){
		return ting::Ref<GamePausedDialog>(new GamePausedDialog());
	}

	inline ting::Ref<Button> ResumeButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->resumeButton)
		return this->dialog->resumeButton;
	}

	inline ting::Ref<Button> BackButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->backButton)
		return this->dialog->backButton;
	}

};



class LevelCompletedDialog : public InGameDialog{
	class Dialog : public InGameDialog::Dialog{
	public:
		ting::Ref<Button> nextLevelButton;
		ting::Ref<Button> restartButton;
		ting::Ref<Button> backButton;
		ting::Ref<LabelWidget> titleLabel;

		Dialog();

		static inline ting::Ref<Dialog> New(){
			return ting::Ref<Dialog>(new Dialog());
		}
	};

	ting::Ref<Dialog> dialog;

	//override
	void OnResize();

public:

	LevelCompletedDialog();

	static inline ting::Ref<LevelCompletedDialog> New(){
		return ting::Ref<LevelCompletedDialog>(new LevelCompletedDialog());
	}

	inline ting::Ref<Button> nextLevelButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->nextLevelButton)
		return this->dialog->nextLevelButton;
	}

	inline ting::Ref<Button> ReplayButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->restartButton)
		return this->dialog->restartButton;
	}

	inline ting::Ref<Button> BackButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->backButton)
		return this->dialog->backButton;
	}

};



class LevelFailedDialog : public InGameDialog{
	class Dialog : public InGameDialog::Dialog{
	public:
		ting::Ref<Button> restartButton;
		ting::Ref<Button> backButton;
		ting::Ref<LabelWidget> titleLabel;

		Dialog();

		static inline ting::Ref<Dialog> New(){
			return ting::Ref<Dialog>(new Dialog());
		}
	};

	ting::Ref<Dialog> dialog;

	//override
	void OnResize();

public:

	LevelFailedDialog();

	static inline ting::Ref<LevelFailedDialog> New(){
		return ting::Ref<LevelFailedDialog>(new LevelFailedDialog());
	}

	inline ting::Ref<Button> ReplayButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->restartButton)
		return this->dialog->restartButton;
	}

	inline ting::Ref<Button> BackButton(){
		ASSERT(this->dialog)
		ASSERT(this->dialog->backButton)
		return this->dialog->backButton;
	}

};

