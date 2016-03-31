/* 
 * File:   Lightning.hpp
 * Author: ivan
 *
 * Created on July 26, 2010, 9:37 PM
 */

#pragma once

#include <ting/Array.hpp>


#include "AnimMan.hpp"
#include "Random.hpp"
#include "ParticleSystem.hpp"
#include "Resources/ResParticleSystem.hpp"
#include "Resources/ResSprite.hpp"
#include "Resources/ResSound.hpp"



class Lightning : public Animation{
	tride::Vec2f scrDim;

	ting::Array<tride::Vec2f> bolt;

	float time;
	float totalTime;

	enum E_State{
		SHADING,
		FLASHING_IN,
		FLASHING_OUT,
		LIGHTENING
	} state;

	void SetState(E_State s);

	void GenerateLightning(tride::Vec2f hitPoint, Random& rnd);

	ting::Ptr<Animation> brokenItemAnim;
public:

	static const float DFadeOutTime = 0.1f;
	static const float DFlashInTime = 0.1f;
	static const float DFlashOutTime = 0.1f;
	static const float DFadeInTime = 0.7f;

	Lightning(
			tride::Vec2f screenDim,
			tride::Vec2f hitPoint,
			ting::Ptr<Animation> brokenItemAnim,
			Random& rnd
		) :
			scrDim(screenDim),
			time(0),
			brokenItemAnim(brokenItemAnim)
	{
		ASS(this->brokenItemAnim)->p = hitPoint;
		this->GenerateLightning(hitPoint, rnd);
		this->SetState(SHADING);
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const;

	//override
	virtual bool Update(float dt);
};



class LightningFactory{
	ting::Ref<ResParticleSystem> hitPar;
	ting::Ref<ResParticleSystem> fragmentFirePar;
	ting::Ref<ResSound> thunderstrikeSnd;
public:
	LightningFactory();

	inline void PlayThunderstrikeSound(){
		this->thunderstrikeSnd->Play();
	}

	ting::Ptr<Lightning> CreateLightning(
			tride::Vec2f screenDim,
			tride::Vec2f hitPoint,
			const ting::Ref<ResSprite> hitItemSprite,
			Random& rnd
		);
};
