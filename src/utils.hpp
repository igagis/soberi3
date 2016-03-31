/*
 * File:   utils.hpp
 * Author: ivan
 *
 * Created on April 25, 2010, 10:31 PM
 */

#pragma once


#include <tride/Vector3.hpp>

#include <ting/Array.hpp>

#include "Random.hpp"
#include "AnimMan.hpp"
#include "Sprite.hpp"
#include "Resources/ResTexture.hpp"
#include "Resources/ResSprite.hpp"
#include "Resources/ResFont.hpp"



ting::Array<Sprite> BreakSprite(const ting::Ref<ResSprite> spr, const tride::Vec2i numPieces);



ting::Ptr<AnimMan> BreakIce(const ting::Ref<ResSprite> spr, const tride::Vec2f& pos, Random& rnd);



class UnlockedLock : public Animation{
	const ting::Ref<ResSprite> sprite;

public:
	UnlockedLock(const ting::Ref<ResSprite> spr, const tride::Vec2f& position);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class Asterisk : public Animation{
	const ting::Ref<ResSprite> sprite;

public:
	Asterisk(const ting::Ref<ResSprite> spr, const tride::Vec2f& position);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class MeltingSnow : public Animation{
	const ting::Ref<ResSprite> sprite;

public:
	MeltingSnow(const ting::Ref<ResSprite> spr, const tride::Vec2f& position);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class TextAnimation : public Animation{
protected:
	const ting::Ref<ResFont> font;

	std::string str;

	tride::Vec3f color;

	float scale;

	TextAnimation(const std::string& str, const tride::Vec3f& color, float scale);
public:
	
};



class SpawningTextAnim : public TextAnimation{
	tride::Vec2f centerMinusOrigin;
public:
	SpawningTextAnim(const tride::Vec2f& dim, const std::string& str, const tride::Vec3f& color, float scale);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};




class LevelCompleted : public TextAnimation{
	const tride::Vec2f dim;
public:
	LevelCompleted(const tride::Vec2f& dim);

	static inline ting::Ptr<LevelCompleted> New(const tride::Vec2f& dim){
		return ting::Ptr<LevelCompleted>(new LevelCompleted(dim));
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class LevelFailed : public SpawningTextAnim{
	const tride::Vec2f dim;
public:
	LevelFailed(const tride::Vec2f& dim);

	static inline ting::Ptr<LevelFailed> New(const tride::Vec2f& dim){
		return ting::Ptr<LevelFailed>(new LevelFailed(dim));
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class GoTextAnim : public SpawningTextAnim{
	const tride::Vec2f dim;
public:
	GoTextAnim(const tride::Vec2f& dim);

	static inline ting::Ptr<GoTextAnim> New(const tride::Vec2f& dim){
		return ting::Ptr<GoTextAnim>(new GoTextAnim(dim));
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class GetReadyTextAnim : public SpawningTextAnim{
	ting::Ptr<GoTextAnim> goTextAnim;
	const tride::Vec2f dim;
public:
	GetReadyTextAnim(const tride::Vec2f& dim);

	static inline ting::Ptr<GetReadyTextAnim> New(const tride::Vec2f& dim){
		return ting::Ptr<GetReadyTextAnim>(new GetReadyTextAnim(dim));
	}

	//override
	virtual bool Update(float dt);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



class HurryUpTextAnim : public TextAnimation{
public:
	HurryUpTextAnim(const tride::Vec2f& dim);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;
};



void RenderRoundedSquare(const tride::Matr4f& matrix, const tride::Vec4f& color, tride::Vec2f dim, float radius);
