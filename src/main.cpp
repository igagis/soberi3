#ifdef M_MAEMO
#include "LibOSSO.hpp"
#endif

#include <ting/debug.hpp>
#include <ting/fs/FSFile.hpp>

#include "MyMaemoFW/Application.hpp"
#include "MyMaemoFW/GLWindow.hpp"
#include "MyMaemoFW/Updater.hpp"
#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/ZipFile.hpp"

#include <aumiks/aumiks.hpp>
#include <aumiks/WavSound.hpp>

#include "ResMan/ResMan.hpp"

#include "App.hpp"
#include "MusicBox.hpp"
#include "GameField.hpp"
#include "GameWindow.hpp"
#include "MainMenuWindow.hpp"
#include "Random.hpp"
#include "Preferences.hpp"



int main(int argc, char *argv[]){
	Application application(argc, argv);

#ifdef M_MAEMO
	LibOSSO libOSSO(
			"com.blogspot.igagis.soberi3",
			"0.0.1"
		);
#endif

	GlobalRandom globalRandom;

	MusicBox musicBox;

	aumiks::Lib aumiksLib(50);

	//Create directory in user's home if it does not exist
	{
		ting::fs::FSFile dir(ting::fs::FSFile::GetHomeDir() + ".forestwalk/");
		if(!dir.Exists()){
//			TRACE(<<"dir does not exist"<<std::endl)
			dir.MakeDir();
		}else{
//			TRACE(<<"dir already exists"<<std::endl)
		}
	}
	Preferences preferences(ting::fs::FSFile::GetHomeDir() + ".forestwalk/" + "preferences.xml");
	preferences.LoadFromFile();

	aumiks::Lib::Inst().SetMuted(Preferences::Inst().IsSoundMuted());
	MusicBox::Inst().SetMuted(Preferences::Inst().IsMusicMuted());

	App app;

	Updater updater;

	GLWindow glWindow;
//	TRACE(<< "main(): showing window" << std::endl)
	glWindow.show();

	resman::ResMan resMan;
//	TRACE(<< "main(): res man created" << std::endl)
#ifdef M_ZIP_RES
	resMan.MountResPackZip("./res.zip");
#else
	resMan.MountResPackDir("./");
#endif
	
	SimpleTexturingShader simpleTexturingShader;
	SimpleColoringShader simpleColoringShader;
	ModulatingColoringTexturingShader modulatingColoringTexturingShader;
	SaturatingColoringTexturingShader saturatingColoringTexturingShader;

//	TRACE(<< "main(): shaders created" << std::endl)

#ifdef M_MAEMO
	glWindow.SetFullscreen(true);
	glWindow.resize(800, 480);
	glWindow.SetPortraitMode(true);
#else
	//TODO:
//	glWindow.resize(480, 800);
	glWindow.resize(800, 480);
	glWindow.SetPortraitMode(true);
#endif
	

//	TRACE(<< "main(): GLWindow size set" << std::endl)

	{
		ting::Ref<MainMenuWindow> mmw(MainMenuWindow::New());
//		TRACE(<< "main(): setting widget" << std::endl)
		glWindow.SetWidget(mmw);
//		TRACE(<< "main(): widget set" << std::endl)
	}

	//connect quit signal
	glWindow.close.Connect(&application, &Application::Quit);

//	TRACE(<< "main(): entering main loop" << std::endl)

	musicBox.Play("mus.mp3");

	int ret = application.Exec();

	preferences.SaveToFile();

	return ret;
}
