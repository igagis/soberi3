/* 
 * File:   ParticleSystem.hpp
 * Author: ivan
 *
 * Created on 10 Август 2010 г., 14:58
 */

#pragma once


#include <list>

#include <ting/types.hpp>
#include <ting/Ref.hpp>
#include <ting/fs/File.hpp>

#include "Random.hpp"

#include "AnimMan.hpp"

#include "Resources/ResSprite.hpp"



class ParticleSystemInfo{
public:
	void Load(ting::fs::File& fi);

	float systemLifeTime;// < 0 means infinte

	//particle lifetime
	float lifeTime;
	float lifeTimeDeviation;

	float startSpeed;
	float startSpeedDeviation;

	float halfSpreadAngle;

	float numParticlesPerSecond;//emission: particles per second

	tride::Vec3f startColor;
	tride::Vec3f endColor;
	float colorDeviation;

	float startAlpha;
	float endAlpha;
	float alphaDeviation;

	float startScale;
	float endScale;
	float scaleDeviation;

	float startRot;
	float endRot;
	float rotDeviation;

	tride::Vec2f gravity;

	enum EBlendMode{
		ADDITIVE,
		BLEND
	} blendMode;
};



class ParticleSystem : public Animation{
	struct Particle{
		tride::Vec2f p;

		tride::Vec2f v;//velocity

		float lifeTime;
		ting::Inited<float, 0> age;

		tride::Vec3f startColor;
		tride::Vec3f colorDelta;

		float startAlpha;
		float alphaDelta;

		float startScale;
		float scaleDelta;

		//TODO: replace rotation with spin
		float startRot;
		float rotDelta;
	};

	typedef std::list<Particle> T_ParList;
	typedef T_ParList::iterator T_ParIter;
	typedef T_ParList::const_iterator T_ParConstIter;

	T_ParList parList;

	ting::Inited<float, 0> systemAge;

	ting::Inited<float, 0> numParticlesToEmit;

	ParticleSystemInfo info;

	ting::Ref<const ResSprite> sprite;

	ting::Inited<bool, false> isStopped;


	inline tride::Vec3f DeviateColor(const tride::Vec3f& clr, Random& rnd)const{
//		TRACE(<< "DeviateColor(): colorDeviation = " << this->info.colorDeviation << std::endl)
		if(this->info.colorDeviation == 0){
//			TRACE(<< "DeviateColor(): colorDeviation is 0" << std::endl)
			return clr;
		}else if(this->info.colorDeviation > 0){
//			TRACE(<< "DeviateColor(): colorDeviation is > 0" << std::endl)
			return clr.CompMul(
					tride::Vec3f(1) - this->info.colorDeviation
							* tride::Vec3f(rnd.RandFloatZeroOne(), rnd.RandFloatZeroOne(), rnd.RandFloatZeroOne())
				);
		}else{//this->info.colorDeviation < 0
//			TRACE(<< "DeviateColor(): colorDeviation is < 0" << std::endl)

			//NOTE: minus is because deviation value is negative
			return clr - (tride::Vec3f(1) - clr).CompMul(
					this->info.colorDeviation
							* tride::Vec3f(rnd.RandFloatZeroOne(), rnd.RandFloatZeroOne(), rnd.RandFloatZeroOne())
				);
		}
	}

public:

	ParticleSystem(const ParticleSystemInfo& info = ParticleSystemInfo()) :
			info(info)
	{}

	inline void SetLifeTime(float t, float deviation = 0){
		this->info.lifeTime = t;
		this->info.lifeTimeDeviation = deviation;
	}

	inline void SetSprite(ting::Ref<const ResSprite> sprite){
		this->sprite = sprite;
	}

	inline void SetStartSpeed(float speed, float deviation = 0){
		this->info.startSpeed = speed;
		this->info.startSpeedDeviation = deviation;
	}

	inline void SetDirection(float dir){
		this->q = dir;
	}

	inline void SetSpread(float spread){
		this->info.halfSpreadAngle = spread / 2;
	}

	inline void SetParticlesEmissionRate(float rate){
		this->info.numParticlesPerSecond = rate;
	}

	inline void SetColors(tride::Vec3f start, tride::Vec3f end, float deviation = 0){
		this->info.startColor = start;
		this->info.endColor = end;
		this->info.colorDeviation = deviation;
	}

	inline void SetAlpha(float start, float end, float deviation = 0){
		this->info.startAlpha = start;
		this->info.endAlpha = end;
		this->info.alphaDeviation = deviation;
	}

	inline void SetScale(float start, float end, float deviation = 0){
		this->info.startScale = start;
		this->info.endScale = end;
		this->info.scaleDeviation = deviation;
	}

	inline void SetRotation(float start, float end, float deviation = 0){
		this->info.startRot = start;
		this->info.endRot = end;
		this->info.rotDeviation = deviation;
	}

	inline void SetBlendMode(ParticleSystemInfo::EBlendMode mode){
		this->info.blendMode = mode;
	}

	inline void SetGravity(tride::Vec2f gravity){
		this->info.gravity = gravity;
	}

	inline void SetSystemLifeTime(float time){
		this->info.systemLifeTime = time;
	}

	inline void Start(){
		this->isStopped = false;
	}

	inline void Stop(){
		this->isStopped = true;
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const;

	//override
	virtual bool Update(float dt);
};
