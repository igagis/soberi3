#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

#include "Application.hpp"

#include "GLWindow.hpp"
#include "Updater.hpp"



Application::Application(int &argc, char *argv[]) :
		QApplication(argc, argv),
		quitFlag(false)
{
	this->setQuitOnLastWindowClosed(false);
}



Application::~Application(){
//	TRACE(<< "Application::~Application(): invoked, num childs = " << this->children().size() << std::endl)
}



int Application::Exec(){
	while(!this->quitFlag){
		Updater::Inst().Update();

		GLWindow::Inst().RenderIfNecessary();

//		TRACE(<< "Application::Exec(): before, qf = " << this->quitFlag << std::endl)

		if(Updater::Inst().ThereIsSomethingToUpdate()){
			this->processEvents();
		}else{
			this->processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);
		}

//		TRACE(<< "Application::Exec(): after, qf = " << this->quitFlag << std::endl)
	}
//	TRACE(<< "Application::Exec(): exit" << std::endl)
	return 0;
}



void Application::Quit(){
//	TRACE(<< "Application::Quit(): invoked" << std::endl)
	this->quitFlag = true;
	this->quit();//this is just to make call to processEvents() unblock in case it is waiting for events
}



void Application::Minimize(){
#ifdef M_MAEMO
	QDBusConnection c = QDBusConnection::sessionBus();
	QDBusMessage m = QDBusMessage::createSignal("/", "com.nokia.hildon_desktop", "exit_app_view");
	c.send(m);
#endif
}



//override
bool Application::event(QEvent* e){
	ASSERT(e)
	
//	TRACE(<< "Application::event(): e->type() = " << (e->type()) << std::endl)
	
	switch(e->type()){
		case QEvent::ApplicationActivate:
			this->activate.Emit();
			break;
		case QEvent::ApplicationDeactivate:
			this->deactivate.Emit();
			break;
		default:
			break;
	}

	return this->QApplication::event(e);
}

