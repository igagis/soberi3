#include <ting/Buffer.hpp>

#include "MyMaemoFW/Shader.hpp"

#include "Random.hpp"

#include "utils.hpp"
#include "Resources/ResFont.hpp"



using namespace ting;
using namespace tride;



ting::Array<Sprite> BreakSprite(const ting::Ref<ResSprite> spr, const tride::Vec2i numPieces){
	ASSERT(spr)

	Vec2f cellDim(
			(spr->GetSprite().verts[3].x - spr->GetSprite().verts[0].x) / float(numPieces.x),
			(spr->GetSprite().verts[1].y - spr->GetSprite().verts[0].y) / float(numPieces.y)
		);
	Vec2f texCellDim(
			(spr->GetSprite().texCoords[3].x - spr->GetSprite().texCoords[0].x) / float(numPieces.x),
			(spr->GetSprite().texCoords[1].y - spr->GetSprite().texCoords[0].y) / float(numPieces.y)
		);

	Vec2f origin = spr->GetSprite().verts[0];
	Vec2f texOrigin = spr->GetSprite().texCoords[0];

	//generate grid of dots
	Array<Array<Vec2f> > verts(numPieces.x + 1);
	Array<Array<Vec2f> > texCoords(numPieces.x + 1);

	ASSERT(verts.Size() == texCoords.Size())

	for(unsigned i = 0; i < verts.Size(); ++i){
		verts[i].Init(numPieces.y + 1);
		texCoords[i].Init(numPieces.y + 1);
		ASSERT(verts[i].Size() == texCoords[i].Size())
		for(unsigned j = 0; j < verts[i].Size(); ++j){
			ASSERT(i < verts.Size())
			ASSERT(j < verts[i].Size())
			verts[i][j] = origin + Vec2f(i * cellDim.x, j * cellDim.y);
			ASSERT(i < texCoords.Size())
			ASSERT(j < texCoords[i].Size())
			texCoords[i][j] = texOrigin + Vec2f(i * texCellDim.x, j * texCellDim.y);
		}
	}


	//generate quads
	Array<Sprite> ret(numPieces.x * numPieces.y);

	ASSERT(numPieces.x > 0)
	for(unsigned i = 0; i < unsigned(numPieces.x); ++i){
		ASSERT(numPieces.y > 0)
		for(unsigned j = 0; j < unsigned(numPieces.y); ++j){
			ASSERT(i * numPieces.y + j < ret.Size())
			Sprite &s = ret[i * numPieces.y + j];

			s.tex = spr->GetSprite().tex;

			ASSERT(3 < s.verts.Size())
			
			ASSERT(i < verts.Size())
			ASSERT(j < verts[i].Size())
			s.verts[0] = verts[i][j];
			ASSERT(i < verts.Size())
			ASSERT(j + 1 < verts[i].Size())
			s.verts[1] = verts[i][j + 1];
			ASSERT(i + 1 < verts.Size())
			ASSERT(j + 1 < verts[i + 1].Size())
			s.verts[2] = verts[i + 1][j + 1];
			ASSERT(i + 1 < verts.Size())
			ASSERT(j < verts[i + 1].Size())
			s.verts[3] = verts[i + 1][j];

			ASSERT(3 < s.texCoords.Size())
			s.texCoords[0] = texCoords[i][j];
			s.texCoords[1] = texCoords[i][j + 1];
			s.texCoords[2] = texCoords[i + 1][j + 1];
			s.texCoords[3] = texCoords[i + 1][j];
		}
	}

	return ret;
}



class IceFragment : public Animation{
	Sprite polySprite;

public:

	tride::Vec2f v;//velocity
	float w;//angular velocity

	IceFragment(const Sprite& ps);

	//override
	virtual void Render(const tride::Matr4f& matrix)const;

	//override
	virtual bool Update(float dt);
};



IceFragment::IceFragment(const Sprite& ps) :
		polySprite(ps)
{
	this->q = 0;

	this->p = tride::Vec2f(0, 0);
	for(unsigned i = 0; i < this->polySprite.verts.Size(); ++i){
		this->p += this->polySprite.verts[i];
	}
	this->p /= ASS(this->polySprite.verts.Size());
	for(unsigned i = 0; i < this->polySprite.verts.Size(); ++i){
		this->polySprite.verts[i] -= this->p;
//		TRACE(<< "vert = " << this->polySprite.verts[i] << std::endl)
	}

	this->totalTime = 2;
}



//override
void IceFragment::Render(const tride::Matr4f& matrix)const{
	float t = this->time / this->totalTime;

	Matr4f matr(matrix);
	matr.Translate(this->p);
	matr.Scale(1 - t);
	matr.Rotate(this->q);

	ASSERT(glGetError() == GL_NO_ERROR)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ASSERT(glGetError() == GL_NO_ERROR)
	glEnable(GL_BLEND);
	ASSERT(glGetError() == GL_NO_ERROR)
	this->polySprite.Render(matr);
	ASSERT(glGetError() == GL_NO_ERROR)
	glDisable(GL_BLEND);
	ASSERT(glGetError() == GL_NO_ERROR)
}



//override
bool IceFragment::Update(float dt){
	this->v += tride::Vec2f(0, -8);
	this->p += this->v * dt;

	this->q += this->w * dt;

	return this->Animation::Update(dt);
}



ting::Ptr<AnimMan> BreakIce(const ting::Ref<ResSprite> spr, const tride::Vec2f& pos, Random& rnd){
	const Array<Sprite> ps = BreakSprite(spr, Vec2i(4, 4));

	ting::Ptr<AnimMan> ret(new AnimMan());

	ret->p = pos;

	for(unsigned i = 0; i < ps.Size(); ++i){
		ting::Ptr<IceFragment> ifr(new IceFragment(ps[i]));

		ifr->v.SetToZero();
		ifr->v.x = ifr->p.x * 2 + (float(rnd.Rand(80)) - 40);
		ifr->v.y = (ifr->p.y - 40) / 2 + float(rnd.Rand(40));
		ifr->w = rnd.RandFloatMinusOne_One() * 15;

		ret->AddAnim(ifr);
	}

	return ret;
}



UnlockedLock::UnlockedLock(const ting::Ref<ResSprite> spr, const tride::Vec2f& position) :
		Animation(position, 0),
		sprite(spr)
{
	this->totalTime = 1;
}



//override
void UnlockedLock::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	float alpha = 1 - t;

	Matr4f matr(matrix);
	matr.Translate(this->p);
	matr.Scale(1 + 2 * t);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->sprite->RenderModulating(matr, Vec4f(1, 1, 1, alpha));
}



Asterisk::Asterisk(const ting::Ref<ResSprite> spr, const tride::Vec2f& position) :
		Animation(position, 0),
		sprite(ASS(spr))
{
	this->totalTime = 1;
}



//override
void Asterisk::Render(const tride::Matr4f& matrix)const{
	Matr4f matr(matrix);
	matr.Translate(this->p);

	float t = this->time / ASSCOND(this->totalTime, > 0);

	matr.Rotate(t);

	if(t < 0.5){
		matr.Scale((t * 2) * 2);
	}else{
		matr.Scale((1 - (t - 0.5) * 2) * 2);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->sprite->Render(matr);
}



MeltingSnow::MeltingSnow(const ting::Ref<ResSprite> spr, const tride::Vec2f& position) :
		Animation(position, 0),
		sprite(spr)
{
	this->totalTime = 3;
}



//override
void MeltingSnow::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	float alpha = 1 - t;

	Matr4f matr(matrix);
	matr.Translate(this->p);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->sprite->RenderModulating(matr, Vec4f(1, 1, 1, alpha));
}



TextAnimation::TextAnimation(const std::string& str, const tride::Vec3f& color, float scale) :
		font(resman::ResMan::Inst().Load<ResFont>("fnt_main")),
		str(str),
		color(color),
		scale(scale)
{}



LevelCompleted::LevelCompleted(const tride::Vec2f& dim) :
		TextAnimation("Completed!", Vec3f(0.3, 1, 0.3), 2),
		dim(dim)
{
	this->totalTime = 2.5;

	this->p.x = (dim.x - this->font->StringBoundingBox(this->str).Width() * this->scale) / 2;
	this->p.y = dim.y / 4;
}



static const float DTextShadingLevel = 0.5f;



//override
void LevelCompleted::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	//shade
	{
		SimpleColoringShader &s = SimpleColoringShader::Inst();
		s.UseProgram();

		s.DisableColorPointer();

		{
			float alpha;
			if(t < 0.5){
				alpha = DTextShadingLevel * t * 2;
			}else{
				alpha = DTextShadingLevel * (1 - t) * 2;
			}
			s.SetColor(Vec4f(0, 0, 0, alpha));
		}

		Matr4f matr(matrix);
		matr.Scale(this->dim);

		s.SetMatrix(matr);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		s.DrawQuad01();
	}

	Matr4f matr(matrix);
	matr.Translate(this->p);
	matr.Translate(0, this->p.y * 2 * t);
	matr.Scale(this->scale, this->scale);

	float alpha;

	const float DThr = 0.3f;
	if(t < DThr){
		alpha = t / ASSCOND(DThr, != 0);
	}else if(t < 1 - DThr){
		alpha = 1;
	}else{
		alpha = 1 - (t - (1 - DThr)) / ASSCOND(DThr, != 0);
	}

	this->font->RenderString(matr, this->str, Vec4f(color, alpha));
}



SpawningTextAnim::SpawningTextAnim(const tride::Vec2f& dim, const std::string& str, const tride::Vec3f& color, float scale) :
		TextAnimation(str, color, scale)
{
	this->p.x = (dim.x - this->font->StringBoundingBox(this->str).Width() * this->scale) / 2;
	this->p.y = (dim.y - this->font->StringBoundingBox(this->str).Height() * this->scale) / 2;

	this->centerMinusOrigin = dim / 2 - this->p;
}



//override
void SpawningTextAnim::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	float factor;

	const float DThr = 0.15f;
	if(t < DThr){
		factor = t / ASSCOND(DThr, != 0);
	}else if(t < 1 - DThr){
		factor = 1;
	}else{
		factor = 1 - (t - (1 - DThr)) / ASSCOND(DThr, != 0);
	}

	Matr4f matr(matrix);
	matr.Translate(this->p);
	matr.Translate(this->centerMinusOrigin * (1 - factor));
	matr.Scale(this->scale * factor);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->font->RenderString(matr, this->str, Vec4f(color, factor));
}



LevelFailed::LevelFailed(const tride::Vec2f& dim) :
		SpawningTextAnim(dim, "Failed!", Vec3f(1, 0.2, 0), 3),
		dim(dim)
{
	this->totalTime = 2;
}



//override
void LevelFailed::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();

	s.DisableColorPointer();

	{
		float alpha;
		if(t < 0.5){
			alpha = DTextShadingLevel * t * 2;
		}else{
			alpha = DTextShadingLevel;
		}
		s.SetColor(Vec4f(0, 0, 0, alpha));
	}

	Matr4f matr(matrix);
	matr.Scale(this->dim);

	s.SetMatrix(matr);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	s.DrawQuad01();

	this->SpawningTextAnim::Render(matrix);
}



GetReadyTextAnim::GetReadyTextAnim(const tride::Vec2f& dim) :
		SpawningTextAnim(dim, "Get Ready!", Vec3f(1, 0.5, 0), 2),
		goTextAnim(GoTextAnim::New(dim)),
		dim(dim)
{
	this->totalTime = 1.5;
}



//override
bool GetReadyTextAnim::Update(float dt){
	if(this->SpawningTextAnim::Update(dt)){
		this->Parent()->AddAnim(this->goTextAnim);
		return true;
	}
	return false;
}



//override
void GetReadyTextAnim::Render(const Matr4f& matrix)const{
	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();

	s.DisableColorPointer();

	s.SetColor(Vec4f(0, 0, 0, DTextShadingLevel));

	Matr4f matr(matrix);
	matr.Scale(this->dim);

	s.SetMatrix(matr);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	s.DrawQuad01();

	this->SpawningTextAnim::Render(matrix);
}



GoTextAnim::GoTextAnim(const tride::Vec2f& dim) :
		SpawningTextAnim(dim, "Go!", Vec3f(1, 0.5, 0), 3),
		dim(dim)
{
	this->totalTime = 1.5;
}



//override
void GoTextAnim::Render(const Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();

	s.DisableColorPointer();

	s.SetColor(Vec4f(0, 0, 0, DTextShadingLevel * (1 - t)));

	Matr4f matr(matrix);
	matr.Scale(this->dim);

	s.SetMatrix(matr);

	s.DrawQuad01();

	this->SpawningTextAnim::Render(matrix);
}



HurryUpTextAnim::HurryUpTextAnim(const tride::Vec2f& dim) :
		TextAnimation("Hurry Up!", Vec3f(1, 0, 0), 2)
{
	this->totalTime = 3;

	this->p.x = (dim.x - this->font->StringBoundingBox(this->str).Width() * this->scale) / 2;
	this->p.y = dim.y / 4;
}



void HurryUpTextAnim::Render(const tride::Matr4f& matrix)const{
	ASSERT(this->time <= this->totalTime)
	ASSERT(this->time >= 0)
	ASSERT(this->totalTime != 0)
	float t = this->time / this->totalTime;

	Matr4f matr(matrix);
	matr.Translate(this->p);
	matr.Translate(0, this->p.y * 2 * t);
	matr.Scale(this->scale, this->scale);

	float alpha;

	const float DThr = 0.3f;
	if(t < DThr){
		alpha = t / ASSCOND(DThr, != 0);
	}else if(t < 1 - DThr){
		alpha = 1;
	}else{
		alpha = 1 - (t - (1 - DThr)) / ASSCOND(DThr, != 0);
	}

	this->font->RenderString(matr, this->str, Vec4f(color, alpha));
}



namespace{

class RoundedCornerBuffer : public StaticBuffer<Vec2f, 12>{
public:
	RoundedCornerBuffer(){
		this->operator[](0) = Vec2f(0, 0);
		this->operator[](1) = Vec2f(0, 1);
		this->operator[](2) = Vec2f(1, 0).Rotate(math::DPi<float>() * 9 / 10);
		this->operator[](3) = Vec2f(1, 0).Rotate(math::DPi<float>() * 8 / 10);
		this->operator[](4) = Vec2f(1, 0).Rotate(math::DPi<float>() * 7 / 10);
		this->operator[](5) = Vec2f(1, 0).Rotate(math::DPi<float>() * 6 / 10);
		this->operator[](6) = Vec2f(1, 0).Rotate(math::DPi<float>() * 5 / 10);
		this->operator[](7) = Vec2f(1, 0).Rotate(math::DPi<float>() * 4 / 10);
		this->operator[](8) = Vec2f(1, 0).Rotate(math::DPi<float>() * 3 / 10);
		this->operator[](9) = Vec2f(1, 0).Rotate(math::DPi<float>() * 2 / 10);
		this->operator[](10) = Vec2f(1, 0).Rotate(math::DPi<float>() * 1 / 10);
		this->operator[](11) = Vec2f(1, 0);
	}
} roundedCorner;

}//~namespace



void RenderRoundedSquare(const tride::Matr4f& matrix, const tride::Vec4f& color, tride::Vec2f dim, float radius){
	ASSERT(radius >= 0)

	if(dim.x < 2 * radius){
		dim.x = 2 * radius;
	}

	if(dim.y < 2 * radius){
		dim.y = 2 * radius;
	}


	SimpleColoringShader &s = SimpleColoringShader::Inst();
	s.UseProgram();

	s.DisableColorPointer();
	s.SetColor(color);


	//render corners

	//bottom-left
	{
		Matr4f matr(matrix);
		matr.Scale(radius);
		matr.Translate(1, 1);
		matr.Rotate(math::DPi<float>());
		
		s.SetMatrix(matr);

		s.EnablePositionPointer();
		s.SetPositionPointer(roundedCorner.Begin());
		s.DrawArrays(GL_TRIANGLE_FAN, roundedCorner.Size());
	}

	//top-left
	{
		Matr4f matr(matrix);
		matr.Translate(0, dim.y - radius);
		matr.Scale(radius);
		matr.Translate(1, 0);
		matr.Rotate(math::DPi<float>() / 2);

		s.SetMatrix(matr);

		s.EnablePositionPointer();
		s.SetPositionPointer(roundedCorner.Begin());
		s.DrawArrays(GL_TRIANGLE_FAN, roundedCorner.Size());
	}

	//top-right
	{
		Matr4f matr(matrix);
		matr.Translate(dim.x - radius, dim.y - radius);
		matr.Scale(radius);

		s.SetMatrix(matr);

		s.EnablePositionPointer();
		s.SetPositionPointer(roundedCorner.Begin());
		s.DrawArrays(GL_TRIANGLE_FAN, roundedCorner.Size());
	}

	//bottom-right
	{
		Matr4f matr(matrix);
		matr.Translate(dim.x - radius, 0);
		matr.Scale(radius);
		matr.Translate(0, 1);
		matr.Rotate(-math::DPi<float>() / 2);

		s.SetMatrix(matr);

		s.EnablePositionPointer();
		s.SetPositionPointer(roundedCorner.Begin());
		s.DrawArrays(GL_TRIANGLE_FAN, roundedCorner.Size());
	}


	//render square parts

	//center
	{
		Matr4f matr(matrix);
		matr.Translate(radius, radius);
		matr.Scale(dim - 2 * Vec2f(radius, radius));

		s.SetMatrix(matr);

		s.DrawQuad01();
	}

	//left
	{
		Matr4f matr(matrix);
		matr.Translate(0, radius);
		matr.Scale(radius, dim.y - 2 * radius);

		s.SetMatrix(matr);

		s.DrawQuad01();
	}

	//right
	{
		Matr4f matr(matrix);
		matr.Translate(dim.x - radius, radius);
		matr.Scale(radius, dim.y - 2 * radius);

		s.SetMatrix(matr);

		s.DrawQuad01();
	}

	//bottom
	{
		Matr4f matr(matrix);
		matr.Translate(radius, 0);
		matr.Scale(dim.x - 2 * radius, radius);

		s.SetMatrix(matr);

		s.DrawQuad01();
	}

	//top
	{
		Matr4f matr(matrix);
		matr.Translate(radius, dim.y - radius);
		matr.Scale(dim.x - 2 * radius, radius);

		s.SetMatrix(matr);

		s.DrawQuad01();
	}
}
