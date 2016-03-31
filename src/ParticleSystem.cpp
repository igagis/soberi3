#include <pugixml/pugixml.hpp>

#include <ting/debug.hpp>

#include "ParticleSystem.hpp"
#include "Random.hpp"



using namespace ting;
using namespace tride;



//override
void ParticleSystem::Render(const tride::Matr4f& matrix)const{
	//set blend mode
	switch(this->info.blendMode){
		case ParticleSystemInfo::ADDITIVE:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case ParticleSystemInfo::BLEND:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		default:
			ASSERT(false)
			break;
	}
	glEnable(GL_BLEND);


	ASSERT(this->sprite)
	for(T_ParConstIter i = this->parList.begin(); i != this->parList.end(); ++i){
		const Particle &par = (*i);

		float t = par.age / ASSCOND(par.lifeTime, > 0);
		ASSERT_INFO(0 <= t && t <= 1, "t = " << t)

		tride::Matr4f matr(matrix);
		matr.Translate(par.p);
		matr.Scale(par.startScale + par.scaleDelta * t);
		matr.Rotate(Vec3f(0, 0, par.startRot + par.rotDelta * t));

		Vec4f color(
				par.startColor + par.colorDelta * t,
				par.startAlpha + par.alphaDelta * t
			);

		this->sprite->RenderModulating(matr, color);
	}
}



//override
bool ParticleSystem::Update(float dt){
//	TRACE(<< "ParticleSystem::Update(): numParticles = " << this->parList.size() << std::endl)
	{
		for(T_ParIter i = this->parList.begin(); i != this->parList.end();){
			Particle &par = (*i);

			par.age += dt;
			if(par.age > par.lifeTime){
				i = this->parList.erase(i);
				continue;
			}

			par.p += par.v * dt;

			par.v += this->info.gravity * dt;
			
			++i;
		}
	}

	if(this->isStopped){
		return this->parList.size() == 0;
	}

	//if particle system has finite life time
	if(this->info.systemLifeTime >= 0){
		this->systemAge += dt;
		if(this->systemAge >= this->info.systemLifeTime){
			this->isStopped = true;
			return false;
		}
	}

	//Emit new particles
	{
		Random &rnd = GlobalRandom::Inst().Rnd();
		
		this->numParticlesToEmit += dt * this->info.numParticlesPerSecond;

		for(; this->numParticlesToEmit >= 1; --this->numParticlesToEmit){
			Particle par;

			par.lifeTime = this->info.lifeTime * (1 - this->info.lifeTimeDeviation * rnd.RandFloatZeroOne());

			par.p = this->p;

			par.v = Vec2f(this->info.startSpeed * (1 - this->info.startSpeedDeviation * rnd.RandFloatZeroOne()), 0)
					.Rotate(this->q + this->info.halfSpreadAngle * rnd.RandFloatMinusOne_One());

			par.startColor = this->DeviateColor(this->info.startColor, rnd);

			par.colorDelta = this->DeviateColor(this->info.endColor, rnd) - par.startColor;

			par.startAlpha = this->info.startAlpha * (1 - this->info.alphaDeviation * rnd.RandFloatZeroOne());
			par.alphaDelta = this->info.endAlpha * (1 - this->info.alphaDeviation * rnd.RandFloatZeroOne()) - par.startAlpha;

			par.startScale = this->info.startScale * (1 - this->info.scaleDeviation * rnd.RandFloatZeroOne());
			par.scaleDelta = this->info.endScale * (1 - this->info.scaleDeviation * rnd.RandFloatZeroOne()) - par.startScale;

			par.startRot = this->info.startRot * (1 - this->info.rotDeviation * rnd.RandFloatZeroOne());
			par.rotDelta = this->info.endRot * (1 - this->info.rotDeviation * rnd.RandFloatZeroOne()) - par.startRot;
			
			this->parList.push_back(par);
		}//~for(this->numParticlesToEmit)
	}

	return false;
}



void ParticleSystemInfo::Load(ting::fs::File &fi){
	ting::Array<ting::u8> rawFile;

	rawFile = fi.LoadWholeFileIntoMemory();

	pugi::xml_document doc;

	if(!doc.load_buffer(rawFile.Begin(), rawFile.Size())){
		throw ting::Exc("ParticleSystemInfo::Load(): error parsing xml");
	}

	pugi::xml_node el = doc.child("ParticleSystem");
	if(el.empty()){
		throw ting::Exc("ParticleSystemInfo::Load(): the file is not a particle system info");
	}

	//spread
	if(pugi::xml_attribute a = el.attribute("spread")){
		this->halfSpreadAngle = a.as_float() / 2;
	}else{
		this->halfSpreadAngle = 0;
	}
	TRACE(<< "ParticleSystemInfo::Load(): spread = " << (this->halfSpreadAngle * 2) << std::endl)

	//lifetime
	if(pugi::xml_attribute a = el.attribute("lifetime")){
		this->lifeTime = a.as_float();
	}else{
		this->lifeTime = 1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): lifetime = " << (this->lifeTime) << std::endl)

	//lifetime deviation
	if(pugi::xml_attribute a = el.attribute("lifetimeDeviation")){
		this->lifeTimeDeviation = a.as_float();
	}else{
		this->lifeTimeDeviation = 0.1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): lifetimeDeviation = " << (this->lifeTimeDeviation) << std::endl)

	//start speed
	if(pugi::xml_attribute a = el.attribute("startSpeed")){
		this->startSpeed = a.as_float();
	}else{
		this->startSpeed = 100;
	}
	TRACE(<< "ParticleSystemInfo::Load(): startSpeed = " << (this->startSpeed) << std::endl)

	//start speed deviation
	if(pugi::xml_attribute a = el.attribute("startSpeedDeviation")){
		this->startSpeedDeviation = a.as_float();
	}else{
		this->startSpeedDeviation = 0.5;
	}
	TRACE(<< "ParticleSystemInfo::Load(): startSpeedDeviation = " << (this->startSpeedDeviation) << std::endl)

	//emission rate
	if(pugi::xml_attribute a = el.attribute("emissionRate")){
		this->numParticlesPerSecond = a.as_float();
	}else{
		this->numParticlesPerSecond = 20;
	}
	TRACE(<< "ParticleSystemInfo::Load(): numParticlesPerSecond = " << (this->numParticlesPerSecond) << std::endl)

	//start color
	if(pugi::xml_attribute a = el.attribute("startColor")){
		this->startColor = Vec3f::ParseXYZ(a.value());
	}else{
		this->startColor = Vec3f(1, 1, 1);
	}
	TRACE(<< "ParticleSystemInfo::Load(): startColor = " << (this->startColor) << std::endl)

	//end color
	if(pugi::xml_attribute a = el.attribute("endColor")){
		this->endColor = Vec3f::ParseXYZ(a.value());
	}else{
		this->endColor = this->startColor;
	}
	TRACE(<< "ParticleSystemInfo::Load(): endColor = " << (this->endColor) << std::endl)

	//color deviation
	if(pugi::xml_attribute a = el.attribute("colorDeviation")){
		this->colorDeviation = a.as_float();
	}else{
		this->colorDeviation = 0.1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): colorDeviation = " << (this->colorDeviation) << std::endl)

	//start alpha
	if(pugi::xml_attribute a = el.attribute("startAlpha")){
		this->startAlpha = a.as_float();
	}else{
		this->startAlpha = 1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): startAlpha = " << (this->startAlpha) << std::endl)

	//end alpha
	if(pugi::xml_attribute a = el.attribute("endAlpha")){
		this->endAlpha = a.as_float();
	}else{
		this->endAlpha = 1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): endAlpha = " << (this->endAlpha) << std::endl)

	//alpha deviation
	if(pugi::xml_attribute a = el.attribute("alphaDeviation")){
		this->alphaDeviation = a.as_float();
	}else{
		this->alphaDeviation = 0.1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): alphaDeviation = " << (this->alphaDeviation) << std::endl)

	//start scale
	if(pugi::xml_attribute a = el.attribute("startScale")){
		this->startScale = a.as_float();
	}else{
		this->startScale = 1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): startScale = " << (this->startScale) << std::endl)

	//end scale
	if(pugi::xml_attribute a = el.attribute("endScale")){
		this->endScale = a.as_float();
	}else{
		this->endScale = 0.5;
	}
	TRACE(<< "ParticleSystemInfo::Load(): endScale = " << (this->endScale) << std::endl)

	//scale deviation
	if(pugi::xml_attribute a = el.attribute("scaleDeviation")){
		this->scaleDeviation = a.as_float();
	}else{
		this->scaleDeviation = 0.1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): scaleDeviation = " << (this->scaleDeviation) << std::endl)

	//blend mode
	if(pugi::xml_attribute a = el.attribute("blendMode")){
		std::string mode(a.value());
		if(mode == "additive"){
			this->blendMode = ParticleSystemInfo::ADDITIVE;
		}else if(mode == "blend"){
			this->blendMode = ParticleSystemInfo::BLEND;
		}else{
			TRACE(<< "ParticleSystemInfo::Load(): unknown blend mode = " << mode.c_str() << std::endl)
			throw ting::Exc("ParticleSystemInfo::Load(): unknown blend mode");
		}
	}else{
		this->blendMode = ParticleSystemInfo::ADDITIVE;
	}
	TRACE(<< "ParticleSystemInfo::Load(): blendMode = " << (this->blendMode) << std::endl)

	//gravity
	if(pugi::xml_attribute a = el.attribute("gravity")){
		this->gravity = Vec2f::ParseXY(a.value());
	}else{
		this->gravity.SetToZero();
	}
	TRACE(<< "ParticleSystemInfo::Load(): gravity = " << (this->gravity) << std::endl)

	//system lifetime
	if(pugi::xml_attribute a = el.attribute("systemLifetime")){
		this->systemLifeTime = a.as_float();
	}else{
		this->systemLifeTime = -1;
	}
	TRACE(<< "ParticleSystemInfo::Load(): systemLifeTime = " << (this->systemLifeTime) << std::endl)
}
