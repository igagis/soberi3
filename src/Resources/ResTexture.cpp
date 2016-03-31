#include "../MyMaemoFW/Image.hpp"

#include "ResTexture.hpp"



using namespace ting;
using namespace ting::fs;



//static
Ref<ResTexture> ResTexture::Load(pugi::xml_node el, File& fi){
	const char *fileName;
	if(pugi::xml_attribute a = el.attribute("file")){
		fileName = a.value();
	}else{
		throw ting::Exc("ResTexture::Load(): there is no \"file\" attribute found");
	}
	ASSERT(fileName)

//	M_RESMANIMPL_LOG(<< "ResTexture::Load(): fileName = " << fileName << std::endl)

	fi.SetPath(fileName);
	Image image(fi);
	image.FlipVertical();
	ASSERT(image.Buf().Size() > 0)

	return Ref<ResTexture>(new ResTexture(image));
}
