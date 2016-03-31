/* 
 * File:   MusicBox.hpp
 * Author: ivan
 *
 * Created on September 13, 2010, 2:26 PM
 */

#pragma once

#include <gst/gst.h>

#include <ting/Singleton.hpp>
#include <ting/Thread.hpp>



class MusicBox : public ting::Singleton<MusicBox>{
	ting::Mutex mutex;

	GstElement* pipeline;

	std::string fileName;

	ting::Inited<bool, false> isMuted;
public:
	MusicBox();

	~MusicBox();

	void Play(const std::string& fileName);

	void Stop();

	void Pause();

	void Resume();

	inline void SetUnpaused(bool unpaused){
		if(unpaused){
			this->Resume();
		}else{
			this->Pause();
		}
	}

	void SetUnmuted(bool unmuted);

	inline void SetMuted(bool muted){
		this->SetUnmuted(!muted);
	}

	void Rewind();
	
private:
	static gboolean BusCallback(GstBus *bus, GstMessage *msg, void *userData);
};


