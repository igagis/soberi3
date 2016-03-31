#include "ResParticleSystem.hpp"



using namespace ting;
using namespace ting::fs;



//static
Ref<ResParticleSystem> ResParticleSystem::Load(pugi::xml_node el, File& fi){
	//get .par filename
	const char *fileName;
	if(pugi::xml_attribute a = el.attribute("file")){
		fileName = a.value();
	}else{
		throw ting::Exc("ResMan::LoadParticleSystem(): there is no \"file\" attribute found");
	}
	ASSERT(fileName)

//	M_RESMANIMPL_LOG(<< "ResMan::LoadParticleSystem(): fileName = " << fileName << std::endl)

	//get sprite
	Ref<ResSprite> sprite;
	if(pugi::xml_attribute a = el.attribute("sprite")){
		sprite = resman::ResMan::Inst().Load<ResSprite>(a.value());
	}else{
		throw ting::Exc("ResMan::LoadParticleSystem(): there is no \"sprite\" attribute found");
	}
	ASSERT(sprite)

	//get direction
	float direction;
	if(pugi::xml_attribute a = el.attribute("dir")){
		direction = a.as_float();
	}else{
		direction = 0;
	}

	fi.SetPath(fileName);
	ParticleSystemInfo info;
	info.Load(fi);

	Ref<ResParticleSystem> resource(new ResParticleSystem(info));

	resource->sprite = sprite;
	resource->dir = direction;

	return resource;
}


