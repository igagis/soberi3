/* 
 * File:   ResFont.cpp
 * Author: ivan
 * 
 * Created on September 19, 2010, 12:46 PM
 */

#include "ResFont.hpp"



using namespace ting;
using namespace ting::fs;



//static
Ref<ResFont> ResFont::Load(pugi::xml_node el, File& fi){
	//get font filename
	const char *fileName;
	if(pugi::xml_attribute a = el.attribute("file")){
		fileName = a.value();
	}else{
		throw ting::Exc("ResFont::Load(): there is no \"file\" attribute found");
	}
	ASSERT(fileName)

//	M_RESMANIMPL_LOG(<< "ResSprite::Load(): texResName = " << texResName << std::endl)

	//read chars attribute
	std::wstring wideChars;
	{
		const char* chars;
		if(pugi::xml_attribute a = el.attribute("chars")){
			chars = a.value();
		}else{
			throw ting::Exc("ResFont::Load(): there is no \"chars\" attribute found");
		}

		//TODO: do utf8 to utf32
		std::string charsStr(chars);
		for(unsigned i = 0; i < charsStr.size(); ++i){
			wideChars += charsStr[i];
		}
	}
	ASSERT(wideChars.size() > 0)

	//read size attribute
	unsigned size;
	if(pugi::xml_attribute a = el.attribute("size")){
		size = a.as_uint();
	}else{
		throw ting::Exc("ResFont::Load(): there is no \"size\" attribute found");
	}

	//read outline attribute
	unsigned outline;
	if(pugi::xml_attribute a = el.attribute("outline")){
		outline = a.as_uint();
	}else{
		outline = 0;
	}

	fi.SetPath(fileName);

	return Ref<ResFont>(new ResFont(fi, wideChars, size, outline));
}

