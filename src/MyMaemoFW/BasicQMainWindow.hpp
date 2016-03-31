#pragma once

#include <QtGui/QMainWindow>
#include <QtGui/QCloseEvent>

#include <ting/Signal.hpp>



class BasicQMainWindow : public QMainWindow{
	Q_OBJECT

	bool isPortrait;
public:
	ting::Signal0 close;
	
	BasicQMainWindow(QWidget *parent = 0, Qt::WindowFlags flags = 0);

	inline bool IsPortraitMode()const{
		return this->isPortrait;
	}

	void SetPortraitMode(bool portrait);

	inline void SetFullscreen(bool fullscreen){
		if(fullscreen){
			this->setWindowState(this->windowState() | Qt::WindowFullScreen);
		}else{
			this->setWindowState(this->windowState() | Qt::WindowStates(~Qt::WindowFullScreen));
		}
	}

protected:
	void PostResizeEvent();

private:
	//override
	virtual void closeEvent(QCloseEvent *event);

	//override
	virtual bool event(QEvent* e);
};


