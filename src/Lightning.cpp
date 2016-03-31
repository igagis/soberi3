#include <vector>

#include "MyMaemoFW/Shader.hpp"

#include "Lightning.hpp"
#include "Sprite.hpp"
#include "utils.hpp"


using namespace ting;
using namespace tride;



void Lightning::SetState(E_State s){
	switch(s){
		case SHADING:
			this->totalTime = DFadeOutTime;
			break;
		case FLASHING_IN:
			this->totalTime = DFlashInTime;
			break;
		case FLASHING_OUT:
			this->totalTime = DFlashOutTime;
			break;
		case LIGHTENING:
			this->totalTime = DFadeInTime;
			break;
		default:
			ASSERT(false)
			break;
	}
	this->state = s;
}


//override
bool Lightning::Update(float dt){
	this->time += dt;

	if(this->time < this->totalTime)
		return false;

	//action has finished

//	this->time -= this->totalTime;
	this->time = 0;

	switch(this->state){
		case SHADING:
			this->SetState(FLASHING_IN);
			break;
		case FLASHING_IN:
			this->SetState(FLASHING_OUT);
			ASS(this->Parent())->AddAnim(this->brokenItemAnim);
			break;
		case FLASHING_OUT:
			this->SetState(LIGHTENING);
			break;
		case LIGHTENING:
			return true;
			break;
		default:
			ASSERT(false)
			break;
	}

	return false;
}



//override
void Lightning::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->totalTime > 0)
	float t = this->time / this->totalTime;

	const float DDarknessLevel = 0.7f;
	const Vec3f DFlashColor(0.95, 0.95, 1);

	Vec3f flashColor;
	float flashAlpha;
	float boltAlpha;
	unsigned numBoltElements;

	switch(this->state){
		case SHADING:
			flashColor = Vec3f(0, 0, 0);
			flashAlpha = DDarknessLevel * t;
			boltAlpha = 1;
			numBoltElements = 2 + (this->bolt.Size() - 2) * t;
			break;
		case FLASHING_IN:
			flashColor = DFlashColor * t;
			flashAlpha = DDarknessLevel + (1 - DDarknessLevel) * t;
			boltAlpha = 1;
			numBoltElements = this->bolt.Size();
			break;
		case FLASHING_OUT:
			flashColor = DFlashColor * (1 - t);
			flashAlpha = DDarknessLevel + (1 - DDarknessLevel) * (1 - t);
			boltAlpha = 1;
			numBoltElements = this->bolt.Size();
			break;
		case LIGHTENING:
			flashColor = Vec3f(0, 0, 0);
			flashAlpha = DDarknessLevel * (1 - t);
			boltAlpha = (1 - t);
			numBoltElements = this->bolt.Size();
			break;
		default:
			ASSERT(false)
			break;
	}


	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();
	s.DisableColorPointer();
	
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	{
		Matr4f matr(matrix);
		matr.Scale(this->scrDim);
		s.SetMatrix(matr);
		s.SetColor(flashColor, flashAlpha);
		s.DrawQuad01();
	}

	s.SetMatrix(matrix);
	s.SetColor(Vec3f(1, 1, 1), boltAlpha);
	s.EnablePositionPointer();
	s.SetPositionPointer(this->bolt.Begin());
	s.DrawArrays(GL_TRIANGLE_STRIP, ASSCOND(numBoltElements, <= this->bolt.Size()));
	glDisable(GL_BLEND);
}



void Lightning::GenerateLightning(tride::Vec2f hitPoint, Random& rnd){
	ASSERT(hitPoint.x >= 0)
	ASSERT(hitPoint.y >= 0)

	Vec2f dir;
	if(hitPoint.y < this->scrDim.y / 2){
		dir = (Vec2f(this->scrDim.x / 2, this->scrDim.y) - hitPoint).Normalize();
	}else{
		dir = (Vec2f(this->scrDim.x / 2, 0) - hitPoint).Normalize();
	}

	Vec2f odir(dir.y, -dir.x);//orthogonal to dir

	ASSERT(this->bolt.Size() == 0)//make sure that thunderbolt is not generated yet

	std::vector<Vec2f> b;
	b.push_back(hitPoint);//start with hit point itself

	Vec2f p;
	Vec2f prevP = hitPoint;
	const float minRnd = 0.3f;
	do{
		p = prevP
				+ dir * (minRnd + (1 - minRnd) * rnd.RandFloatZeroOne()) * 40
				+ odir * rnd.RandFloatMinusOne_One() * 30;
		b.push_back(p);
		prevP = p;
	}while(0 < p.y && p.y < this->scrDim.y);

	this->bolt.Init(b.size() * 2);

	//reverse thunderbolt and generate triangle strip
	{
		const float minWidth = 4;
		const float maxWidth = 10;
		Vec2f odir(b[1].y < b[0].y ? (-1) : (1), 0);
		for(unsigned i = 0; i < b.size(); ++i){
			float hw = minWidth + (maxWidth - minWidth) * (float(i) / float(b.size()));

			this->bolt[ASSCOND(this->bolt.Size() - 1 - i * 2, < this->bolt.Size())]
					= b[i] + hw * odir;
			this->bolt[ASSCOND(this->bolt.Size() - 2 - i * 2, < this->bolt.Size())]
					= b[i] - hw * odir;
		}
	}
}



LightningFactory::LightningFactory(){
	this->hitPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_lightning_hit");
	this->fragmentFirePar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_fragment_fire");
	this->thunderstrikeSnd = resman::ResMan::Inst().Load<ResSound>("snd_thunderstrike");
}



class ItemFragment : public Animation{
	Sprite sprite;

	Ptr<ParticleSystem> ps;

	ting::Inited<float, 0> age;
	float lifetime;
public:
	tride::Vec2f v;//velocity
	float w;//angular velocity

	ItemFragment(
			const Sprite& sprite,
			Ptr<ParticleSystem> ps
		) :
			sprite(sprite),
			ps(ps)
	{
		this->lifetime = 1;

		this->p = tride::Vec2f(0, 0);
		for(unsigned i = 0; i < this->sprite.verts.Size(); ++i){
			this->p += this->sprite.verts[i];
		}
		this->p /= ASS(this->sprite.verts.Size());
		for(unsigned i = 0; i < this->sprite.verts.Size(); ++i){
			this->sprite.verts[i] -= this->p;
	//		TRACE(<< "vert = " << this->sprite.verts[i] << std::endl)
		}

		ASS(this->ps)->p = this->p;
	}

	//override
	virtual void Render(const tride::Matr4f& matrix)const{
		float t = this->age / this->lifetime;
		ASSERT(0 <= t && t <= 1)

		Matr4f matr(matrix);
		matr.Translate(this->p);
		matr.Scale((1 - t) * 2);
		matr.Rotate(this->q);

		ASSERT(glGetError() == GL_NO_ERROR)
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		ASSERT(glGetError() == GL_NO_ERROR)
		glEnable(GL_BLEND);
		ASSERT(glGetError() == GL_NO_ERROR)
		this->sprite.Render(matr);
		ASSERT(glGetError() == GL_NO_ERROR)

		ASS(this->ps)->Render(matrix);
	}

	//override
	virtual bool Update(float dt){
		bool psret = ASS(this->ps)->Update(dt);

		if(this->age == this->lifetime)
			return psret;
		
		this->age += dt;

		if(this->age >= this->lifetime){
			this->age = this->lifetime;
			ASS(this->ps)->Stop();
		}

		this->v += tride::Vec2f(0, -8);
		this->p += this->v * dt;

		this->q += this->w * dt;

		ASS(this->ps)->p = this->p;

		return false;
	}
};



static ting::Ptr<AnimMan> BreakItem(const ting::Ref<ResSprite> spr, Ref<ResParticleSystem> firePar, Random& rnd){
	const Array<Sprite> ps = BreakSprite(spr, Vec2i(2, 2));

	ting::Ptr<AnimMan> ret(new AnimMan());

	for(unsigned i = 0; i < ps.Size(); ++i){
		ting::Ptr<ItemFragment> ifr(new ItemFragment(ps[i], ASS(firePar)->CreateParticleSystem()));

		ifr->v = ifr->p * 10;
		ifr->w = rnd.RandFloatMinusOne_One() * 2;

		ret->AddAnim(ifr);
	}

	return ret;
}



ting::Ptr<Lightning> LightningFactory::CreateLightning(
		tride::Vec2f screenDim,
		tride::Vec2f hitPoint,
		const ting::Ref<ResSprite> hitItemSprite,
		Random& rnd
	)
{
	Ptr<AnimMan> a = BreakItem(hitItemSprite, this->fragmentFirePar, rnd);
	ASS(a)->AddAnim(ASS(this->hitPar)->CreateParticleSystem());

	return Ptr<Lightning>(new Lightning(
			screenDim,
			hitPoint,
			a,
			rnd
		));
}
