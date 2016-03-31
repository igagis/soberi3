#include "App.hpp"

#include <aumiks/aumiks.hpp>

#include "MyMaemoFW/Application.hpp"

#ifdef M_MAEMO
#include "LibOSSO.hpp"
#endif

#include "MusicBox.hpp"
#include "Preferences.hpp"



App::App(){
	Application::Inst().activate.Connect(this, &App::UnmuteAudio);
	Application::Inst().deactivate.Connect(this, &App::MuteAudio);
#ifdef M_MAEMO
	LibOSSO::Inst().wentToSleep.Connect(this, &App::MuteAudio);
#endif
}



App::~App(){
	Application::Inst().activate.Disconnect(this, &App::UnmuteAudio);
	Application::Inst().deactivate.Disconnect(this, &App::MuteAudio);
#ifdef M_MAEMO
	LibOSSO::Inst().wentToSleep.Disconnect(this, &App::MuteAudio);
#endif
}



void App::MuteAudio(){
	aumiks::Lib::Inst().Mute();
	MusicBox::Inst().SetMuted(true);
}



void App::UnmuteAudio(){
	aumiks::Lib::Inst().SetMuted(Preferences::Inst().IsSoundMuted());
	MusicBox::Inst().SetMuted(Preferences::Inst().IsMusicMuted());
}


