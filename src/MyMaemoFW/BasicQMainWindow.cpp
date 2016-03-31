#include <QWindowStateChangeEvent>

#include "BasicQMainWindow.hpp"


BasicQMainWindow::BasicQMainWindow(QWidget *parent, Qt::WindowFlags flags) :
		QMainWindow(parent, flags)
{
	this->setAttribute(Qt::WA_QuitOnClose, false);
	this->setAttribute(Qt::WA_DeleteOnClose, false);

	//set initial mode, this will also initialize this->isPortrait variable.
	this->SetPortraitMode(false);
}



//override
void BasicQMainWindow::closeEvent(QCloseEvent *event){
//	TRACE(<< "BasicQMainWindow::closeEvent(): invoked" << std::endl)
	ASS(event)->accept();
	this->close.Emit();
}



void BasicQMainWindow::SetPortraitMode(bool portrait){
#ifdef Q_WS_MAEMO_5
	if(portrait){
		this->setAttribute(Qt::WA_Maemo5PortraitOrientation, true);
	}else{
		this->setAttribute(Qt::WA_Maemo5LandscapeOrientation, true);
	}
#endif
	this->isPortrait = portrait;

	//post resize event to reinit GL viewport and other stuff related to window size
	this->PostResizeEvent();
}



void BasicQMainWindow::PostResizeEvent(){
	int w = this->width();
	int h = this->height();

	this->resize(w + 1, h + 1);//if call with same width and height Qt will not post the event
	this->resize(w, h);
}



//override
bool BasicQMainWindow::event(QEvent* e){
	ASSERT(e)

//	TRACE(<< "BasicQMainWindow::event(): e->type() = " << (e->type()) << std::endl)

	return this->QMainWindow::event(e);
}
