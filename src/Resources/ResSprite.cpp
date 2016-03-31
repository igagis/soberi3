#include "../MyMaemoFW/Shader.hpp"

#include "ResSprite.hpp"



using namespace ting;
using namespace ting::fs;
using namespace tride;



//static
Ref<ResSprite> ResSprite::Load(pugi::xml_node el, File& fi){
	const char *texResName;
	if(pugi::xml_attribute a = el.attribute("tex")){
		texResName = a.value();
	}else{
		throw ting::Exc("ResSprite::Load(): there is no \"tex\" attribute found");
	}
	ASSERT(texResName)

//	M_RESMANIMPL_LOG(<< "ResSprite::Load(): texResName = " << texResName << std::endl)

	Ref<ResTexture> tex = resman::ResMan::Inst().Load<ResTexture>(texResName);
	ASSERT(tex.IsValid())

	//read xy attribute
	Vec2f xy;
	if(pugi::xml_attribute a = el.attribute("xy")){
		xy = Vec2f::ParseXY(a.value());
	}else{
		throw ting::Exc("ResSprite::Load(): there is no \"xy\" attribute found");
	}

	//read wh attribute
	Vec2f wh;
	if(pugi::xml_attribute a = el.attribute("wh")){
		wh = Vec2f::ParseXY(a.value());
	}else{
		throw ting::Exc("ResSprite::Load(): there is no \"wh\" attribute found");
	}

	//read dim (dimensions) attribute
	Vec2f dim;
	if(pugi::xml_attribute a = el.attribute("dim")){
		dim = Vec2f::ParseXY(a.value());
	}else{
		dim = wh;
	}

	//read pivot attribute
	Vec2f pivot;
	if(pugi::xml_attribute a = el.attribute("pivot")){
		pivot = Vec2f::ParseXY(a.value());
	}else{
		pivot.SetToZero();
	}

	return Ref<ResSprite>(new ResSprite(tex, xy, wh, dim, pivot));
}



ResSprite::ResSprite(
		const ting::Ref<ResTexture> &texture,
		const tride::Vec2f& xy,
		const tride::Vec2f& wh,
		const tride::Vec2f& dimensions,
		const tride::Vec2f& spritePivot
	) :
		dim(dimensions),
		pivot(spritePivot)
{
//	TRACE(<< "ResSprite::ResSprite(): tex->Dim() = " << this->tex->Dim() << std::endl)

	this->spr.tex = texture;

	//init quad
	this->spr.verts[0] = tride::Vec2f(0, 0) - this->pivot;
	this->spr.texCoords[0] = Vec2f(xy.x / this->spr.tex->Dim().x, (this->spr.tex->Dim().y - (xy.y + wh.y)) / this->spr.tex->Dim().y);

	this->spr.verts[1] = tride::Vec2f(0, this->dim.y) - this->pivot;
	this->spr.texCoords[1] = Vec2f(xy.x / this->spr.tex->Dim().x, (this->spr.tex->Dim().y - xy.y) / this->spr.tex->Dim().y);

	this->spr.verts[2] = tride::Vec2f(this->dim.x, this->dim.y) - this->pivot;
	this->spr.texCoords[2] = Vec2f((xy.x + wh.x) / this->spr.tex->Dim().x, (this->spr.tex->Dim().y - xy.y) / this->spr.tex->Dim().y);

	this->spr.verts[3] = tride::Vec2f(this->dim.x, 0) - this->pivot;
	this->spr.texCoords[3] = Vec2f((xy.x + wh.x) / this->spr.tex->Dim().x, (this->spr.tex->Dim().y - (xy.y + wh.y)) / this->spr.tex->Dim().y);
}



void ResSprite::Scale(tride::Vec2f scale){
	this->spr.Scale(scale);

	this->dim.x *= scale.x;
	this->dim.y *= scale.y;

	this->pivot.x *= scale.x;
	this->pivot.y *= scale.y;
}

