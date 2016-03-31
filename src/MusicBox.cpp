/* 
 * File:   MusicBox.cpp
 * Author: ivan
 * 
 * Created on September 13, 2010, 2:26 PM
 */

#include "MusicBox.hpp"
#include "AnimMan.hpp"



using namespace ting;



MusicBox::MusicBox() :
		pipeline(0)
{
	gst_init(0, 0);
}



MusicBox::~MusicBox(){
	this->Stop();
}



void MusicBox::Stop(){
	ting::Mutex::Guard mutexGuard(this->mutex);
	
	if(!this->pipeline)
		return;

	gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(this->pipeline));

	this->pipeline = 0;
}



void MusicBox::Play(const std::string& fileName){
	this->Stop();

	this->fileName = fileName;

	{
		ting::Mutex::Guard mutexGuard(this->mutex);

		ASSERT(!this->pipeline)

		{
			GError *error = NULL;
			this->pipeline = gst_parse_launch("filesrc name=file_src_element ! decodebin ! audioconvert ! audioresample ! autoaudiosink", &error);
			if(!this->pipeline){
				TRACE(<< "MusicBox::Play(): failed paring the pipeline: " << error->message << std::endl)
				return;
			}
		}

		{
			GstElement *filesrc = gst_bin_get_by_name(GST_BIN(this->pipeline), "file_src_element");
			if(!filesrc){
				TRACE(<< "MusicBox::Play(): could not find filesrc" << std::endl)
				gst_object_unref(GST_OBJECT(this->pipeline));
				this->pipeline = 0;
				return;
			}
			g_object_set(G_OBJECT(filesrc), "location", fileName.c_str(), NULL);
		}

		{
			GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
			gst_bus_add_watch(bus, &MusicBox::BusCallback, this);
			gst_object_unref(bus);
		}


		gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PLAYING);
	}

	this->SetMuted(this->isMuted);
}



void MusicBox::Rewind(){
	ting::Mutex::Guard mutexGuard(this->mutex);

	if(!this->pipeline)
		return;

	if(!gst_element_seek(
			this->pipeline,
			1.0, //playback rate remains normal
			GST_FORMAT_TIME,
			GST_SEEK_FLAG_FLUSH,
			GST_SEEK_TYPE_SET, //set absolute position
			0, //new position (beginning of the track
			GST_SEEK_TYPE_NONE, //no new stop position
			GST_CLOCK_TIME_NONE
		))
	{
		TRACE(<< "MusicBox::Rewind(): gst_element_seek() failed" << std::endl)
		return;
	}
}



void MusicBox::Pause(){
	TRACE(<< "MusicBox::Pause(): invoked" << std::endl)

	ting::Mutex::Guard mutexGuard(this->mutex);

	if(!this->pipeline)
		return;

	gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PAUSED);
}



void MusicBox::Resume(){
	TRACE(<< "MusicBox::Resume(): invoked" << std::endl)

	ting::Mutex::Guard mutexGuard(this->mutex);

	if(!this->pipeline)
		return;

	gst_element_set_state(GST_ELEMENT(this->pipeline), GST_STATE_PLAYING);
}



//static
gboolean MusicBox::BusCallback(GstBus* bus, GstMessage* msg, void* userData){
//	TRACE(<< "MusicBox::BusCallback(): enter" << std::endl)
	MusicBox &mb = *reinterpret_cast<MusicBox*>(userData);

	switch(GST_MESSAGE_TYPE(msg)){
		case GST_MESSAGE_EOS:
		{
			TRACE(<< "MusicBox::BusCallback(): EOS" << std::endl)
			mb.Rewind();
			break;
		}

		case GST_MESSAGE_ERROR:
		{
			gchar *debug;
			GError *err;

			gst_message_parse_error(msg, &err, &debug);
			g_free(debug);
			TRACE(<< "MusicBox::BusCallback(): ERROR: " << err->message << std::endl)
			g_error_free(err);
			break;
		}

		default:
			break;
	}

	return true;
}



void MusicBox::SetUnmuted(bool unmuted){
	this->isMuted = !unmuted;

	this->SetUnpaused(unmuted);
}

