/* 
 * File:   ResParticleSystem.hpp
 * Author: ivan
 *
 * Created on August 27, 2010, 8:48 PM
 */

#pragma once


#include "../ResMan/ResMan.hpp"

#include "../ParticleSystem.hpp"



class ResParticleSystem : public resman::Resource{
	friend class resman::ResMan;

	const ParticleSystemInfo info;

	ting::Ref<const ResSprite> sprite;

	float dir;

	ResParticleSystem(const ParticleSystemInfo& info) :
			info(info)
	{}
public:

	inline ting::Ptr<ParticleSystem> CreateParticleSystem()const{
		ting::Ptr<ParticleSystem> ret(new ParticleSystem(this->info));
		ret->SetSprite(this->sprite);
		ret->q = this->dir;
		return ret;
	}

private:
	static ting::Ref<ResParticleSystem> Load(pugi::xml_node el, ting::fs::File &fi);
};
