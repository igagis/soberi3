#pragma once

#include <QtGui/QApplication>

#include <ting/Singleton.hpp>
#include <ting/Signal.hpp>



class Application : public QApplication, public ting::Singleton<Application>{
	Q_OBJECT

	bool quitFlag;
public:
	ting::Signal0 activate;
	ting::Signal0 deactivate;


	//NOTE: argc is passed as reference!!!
	//      If passing by value, the value is lost and it causes crash later!
	Application(int &argc, char *argv[]);


	~Application();


	int Exec();


	void Quit();


	void Minimize();


	//override
	virtual bool event(QEvent* e);
};

