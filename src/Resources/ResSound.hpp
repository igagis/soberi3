/* 
 * File:   ResSound.hpp
 * Author: ivan
 *
 * Created on August 27, 2010, 8:44 PM
 */

#pragma once


#include "../ResMan/ResMan.hpp"

#include <aumiks/WavSound.hpp>



class ResSound : public resman::Resource{
	friend class resman::ResMan;

	const ting::Ref<aumiks::WavSound> snd;

	ResSound(const ting::Ref<aumiks::WavSound> &sound) :
			snd(sound)
	{
		ASSERT(this->snd.IsValid())
	}
public:

	inline ting::Ref<aumiks::WavSound::Channel> CreateChannel()const{
		return this->snd->CreateWavChannel();
	}

	inline ting::Ref<aumiks::WavSound::Channel> Play()const{
		return this->snd->Play();
	}

private:
	static ting::Ref<ResSound> Load(pugi::xml_node el, ting::fs::File &fi);
};


