#include "ResSound.hpp"



using namespace ting;
using namespace ting::fs;



//static
Ref<ResSound> ResSound::Load(pugi::xml_node el, File& fi){
	const char *fileName;
	if(pugi::xml_attribute a = el.attribute("file")){
		fileName = a.value();
	}else{
		throw ting::Exc("ResSound::Load(): there is no \"file\" attribute found");
	}
	ASSERT(fileName)

//	M_RESMANIMPL_LOG(<< "ResSound::Load(): fileName = " << fileName << std::endl)

	fi.SetPath(fileName);
	Ref<aumiks::WavSound> snd = aumiks::WavSound::LoadWAV(fi);

	return Ref<ResSound>(new ResSound(ASS(snd)));
}
