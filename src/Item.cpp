#include <GLES2/gl2.h>

#include "utils.hpp"
#include "Item.hpp"
#include "GameField.hpp"
#include "Lightning.hpp"



using namespace ting;
using namespace ting::math;
using namespace tride;



void Item::StartSpawning(){
	ASSERT(this->IsIdle())
	this->SetActTime(DItemFallingTime() * 1.2);
	this->SetState(SPAWNING);
}



static const float DStoppingDistance = 10;



void Item::StartStopping(){
	ASSERT(this->IsIdle())
	ASSERT(this->hasJustStoppedAction == Item::MOVING)
	this->SetState(Item::STOPPING_1);
	this->totalActTime = 0.1f;
	this->movingDelta = Vec2f(0, -DStoppingDistance);
}



void Item::StartHitByLightning(){
	ASSERT(this->IsIdle())
	this->SetState(Item::HIT_BY_LIGHTNING_1);
	this->SetActTime(Lightning::DFadeOutTime + Lightning::DFlashInTime);
}



void Item::StartHinting(tride::Vec2i dir){
	ASSERT(
			(Abs(dir.x) == 1 && Abs(dir.y) == 0) ||
			(Abs(dir.x) == 0 && Abs(dir.y) == 1)
		)

	ASSERT(this->IsIdle())
	this->SetState(Item::IDLE_HINTING_1);
	this->SetActTime(0.3f);
	this->movingDelta = Vec2f(dir.x, dir.y) * 10;
}



void Item::StartSwapping(Cell* from, Cell* to){
	ASSERT(from)
	ASSERT(to)
	ASSERT(this->IsIdle())
	this->swappingCell = from;
	this->movingDelta = from->pos - to->pos;
	this->SetActTime(DItemSwappingTime());
	this->SetState(SWAPPING);
}



void Item::StartSwappingBack(Cell* from, Cell* to){
	ASSERT(from)
	ASSERT(to)
	ASSERT(this->IsIdle())
	this->movingDelta = from->pos - to->pos;
	TRACE(<< "Item::StartSwappingBack(): moving delta = " << this->movingDelta << std::endl)
	this->SetActTime(DItemSwappingTime());
	this->SetState(SWAPPING_BACK);
}



Item::Item(E_Type t) :
		actTime(0),
		totalActTime(1),
		vel(0),
		movingDelta(0, 0),
		type(t),
		state(IDLE),
		hasJustStoppedAction(IDLE),
		swappingCell(0)
{}



//static
Ref<ResSprite> PopItem::LoadSpriteForItem(E_Type type){
	switch(type){
		case ZERO:
			return resman::ResMan::Inst().Load<ResSprite>("spr_cornflower");
			break;
		case ONE:
			return resman::ResMan::Inst().Load<ResSprite>("spr_blueberry");
			break;
		case TWO:
			return resman::ResMan::Inst().Load<ResSprite>("spr_daizy");
			break;
		case THREE:
			return resman::ResMan::Inst().Load<ResSprite>("spr_mushroom");
			break;
		case FOUR:
			return resman::ResMan::Inst().Load<ResSprite>("spr_acorn");
			break;
		case FIVE:
			return resman::ResMan::Inst().Load<ResSprite>("spr_strawberry");
			break;
		case SIX:
			return resman::ResMan::Inst().Load<ResSprite>("spr_pinecone");
			break;
		default:
			ASSERT(false)
			break;
	}
	return 0;
}



PopItem::PopItem(PopItem::E_Type t, bool isGoal) :
		Item(Item::POPITEM),
		type(t),
		isGoalItem(isGoal)
{
	this->sprite = PopItem::LoadSpriteForItem(this->type);
	ASSERT(this->sprite)

	switch(this->type){
		case ZERO:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop0");
			break;
		case ONE:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop1");
			break;
		case TWO:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop2");
			break;
		case THREE:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop3");
			break;
		case FOUR:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop4");
			break;
		case FIVE:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop5");
			break;
		case SIX:
			this->popPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_pop6");
			break;
		default:
			ASSERT(false)
			break;
	}
}



/*
static Vec3f itemColor[] = {
	Vec3f(0, 1, 0), //0xff00ff00,
	Vec3f(1, 0, 1), //0xffff00ff,
	Vec3f(1, 0, 0), //0xffff0000,
	Vec3f(0, 0, 1), //0xff0000ff,
	Vec3f(0, 1, 1), //0xff00ffff,
	Vec3f(1, 1, 0), //0xffffff00,
	Vec3f(0.2, 0.2, 0.2), //0xff808080,
	Vec3f(1, 1, 1) //0xffffffff
};
*/



//override
void PopItem::Render(const tride::Matr4f& matrix)const{
	switch(this->state){
		case Item::HIT_BY_LIGHTNING_2:
			return;//do not render anything
			break;
		default:
			break;
	}

	ASSERT(this->totalActTime != 0)
	float t = this->actTime / this->totalActTime;

	tride::Matr4f matr(matrix);

	switch(this->state){
		case Item::MOVING:
		case Item::SWAPPING:
		case Item::SWAPPING_BACK:
		case Item::IDLE_HINTING_2:
		case Item::STOPPING_2:
			matr.Translate(
					this->movingDelta * (1 - t)
				);
			break;
		case Item::IDLE_HINTING_1:
		case Item::STOPPING_1:
			matr.Translate(
					this->movingDelta * t
				);
			break;
		default:
			break;
	}

	switch(this->state){
		case Item::SPAWNING:
			matr.Scale(t);
			break;
		case Item::POPPING:
			{
				const float thr = 0.2;
				if(t < thr){
					matr.Scale(1 + t);
				}else{
					matr.Scale(
							(1 - (t - thr) / (1 - thr)) * (1 + thr)
						);
				}
			}
			break;
		case Item::STOPPING_1:
			{
				float k = 2 * DStoppingDistance * (1 + this->vel) * t;
				matr.Scale(
						1 + k / this->sprite->Dim().x,
						1 - k / this->sprite->Dim().y
					);
			}
			break;
		case Item::STOPPING_2:
			{
				float k = 2 * DStoppingDistance * (1 + this->vel) * (1 - t);
				matr.Scale(
						1 + k / this->sprite->Dim().x,
						1 - k / this->sprite->Dim().y
					);
			}
			break;
		default:
			break;
	}

	if(this->isGoalItem){
/*
		GameField::Inst().goalItemAuraSpr->SetColor(0xffffff00);
		GameField::Inst().goalItemAuraSpr->Render(
					pos,
					CGF::Inst().Hge()->Timer_GetTime(),
					scale * (2 + 0.3f * sin(2 * CGF::Inst().Hge()->Timer_GetTime()) )
				);
*/
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	this->sprite->Render(matr);
}



IceCubeItem::IceCubeItem() :
		Item(Item::ICECUBE),
		cracks(NO_CRACKS)
{
	this->asteriskSpr = resman::ResMan::Inst().Load<ResSprite>("spr_asterisk");

	this->noCracksSpr = resman::ResMan::Inst().Load<ResSprite>("spr_ice_cube");
	this->lightCracksSpr = resman::ResMan::Inst().Load<ResSprite>("spr_ice_cube_light_cracks");
	this->heavyCracksSpr = resman::ResMan::Inst().Load<ResSprite>("spr_ice_cube_heavy_cracks");
}



//override
bool IceCubeItem::SpecialUpdate(float dt){
//	TRACE(<< "IceCubeItem::SpecificUpdate(): invoked" << std::endl)
	this->anim.Update(dt);

	const float DAsteriskTimeStep = 0.2;//0.2 second
	const float DProbabilityPerSecond = 0.1;

	this->asteriskTime += dt;
	while(this->asteriskTime > 0){
		this->asteriskTime -= DAsteriskTimeStep;
		if(
				GlobalRandom::Inst().Rnd().RandFloatZeroOne()
						< (1 - math::Exp(DAsteriskTimeStep * math::Ln(1 - DProbabilityPerSecond)))
			)
		{
			this->anim.AddAnim(Ptr<Animation>(
					new Asterisk(ASS(this->asteriskSpr), Vec2f(28, 30))
				));
//			TRACE(<< "IceCubeItem::SpecificUpdate(): asterisk generated, num anims = " << this->anim.Size() << std::endl)
		}
	}
	
	return false;
}



bool IceCubeItem::Hit(){
	switch(this->cracks){
		case NO_CRACKS:
			this->cracks = LIGHT_CRACKS;
			break;
		case LIGHT_CRACKS:
			this->cracks = HEAVY_CRACKS;
			break;
		case HEAVY_CRACKS:
			this->cracks = DESTROYED;
			this->StartPopping(0.2f);
			return true;
			break;
		case DESTROYED:
			//do nothing
			break;
		default:
			ASSERT(false)
			break;
	}
	return false;
}



//override
void IceCubeItem::Render(const tride::Matr4f& matrix)const{
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	switch(this->cracks){
		case NO_CRACKS:
			ASS(this->noCracksSpr)->Render(matrix);
			break;
		case LIGHT_CRACKS:
			ASS(this->lightCracksSpr)->Render(matrix);
			break;
		case HEAVY_CRACKS:
			ASS(this->heavyCracksSpr)->Render(matrix);
			break;
		case DESTROYED:
			break;
		default:
			ASSERT(false)
			break;
	}

	this->anim.Render(matrix);
}



void Item::SetState(E_State state){
//	switch (state)
//	{
//		case IDLE:
//			break;
//		case SPAWNING:
//			break;
//		case POPPING:
//			break;
//		case MOVING:
//			break;
//		case SWAPPING:
//			break;
//		case SWAPPING_BACK:
//			break;
//		case FINISHED_SWAPPING:
//			ASSERT(this->swappingCell)
//			break;
//		default:
//			ASSERT_INFO(false, "Item::SetState(): unknown state requested");
//			break;
//	}
	this->state = state;
}



bool Item::Update(float dt){
	if(this->SpecialUpdate(dt)){
		//item will be deleted, no need to do the rest of update
		return true;
	}

	if(this->state == Item::IDLE){
		return false;
	}

	bool actFinished = false;
	bool ret = false;

	if(this->state == Item::MOVING){
		if(this->vel < 1.5){
			const float acc = 7;
			this->vel += dt * acc;
		}else{
			this->vel = 1.5;
		}
		this->actTime += dt * this->vel;
	}else{
		this->actTime += dt;
	}

	if(this->actTime >= this->totalActTime){
		this->actTime -= this->totalActTime;
		actFinished = true;
	}

	switch(this->state){
		case IDLE:
			break;
		case SPAWNING:
			if(actFinished){
				this->hasJustStoppedAction = SPAWNING;
			}
			break;
		case HIT_BY_LIGHTNING_2:
		case POPPING:
			ret = actFinished;
			break;
		case MOVING:
			if(actFinished){
				this->movingDelta.SetToZero();
				this->hasJustStoppedAction = MOVING;
			}
			break;
		case STOPPING_1:
			if(actFinished){
				this->SetState(STOPPING_2);
				this->actTime = 0;//to make stopping more noticable
				actFinished = false;
			}
			break;
		case STOPPING_2:
			if(actFinished){
				this->actTime = 0;
			}
			break;
		case IDLE_HINTING_1:
			if(actFinished){
				this->SetState(IDLE_HINTING_2);
				this->actTime = 0;//to make hinting more noticable
				actFinished = false;
			}
			break;
		case IDLE_HINTING_2:
			if(actFinished){
				this->actTime = 0;
			}
			break;
		case SWAPPING:
			if(actFinished){
				this->SetState(FINISHED_SWAPPING);
				this->movingDelta.SetToZero();
				this->actTime = 0;
				actFinished = false;//clear this flag so the item will not pass to IDLE state right after this switch
			}
			break;
		case SWAPPING_BACK:
			if(actFinished){
				this->movingDelta.SetToZero();
				this->actTime = 0;
			}
			break;
		case FINISHED_SWAPPING:
			//this state should not be changed by actFinished
			actFinished = false;
			this->actTime = 0;
			this->totalActTime = 1;
			break;
		case HIT_BY_LIGHTNING_1:
			if(actFinished){
				this->SetState(HIT_BY_LIGHTNING_2);
				this->SetActTime(0.2f);
				actFinished = false;
			}
			break;
		default:
			break;
	}

	if(actFinished)
		this->SetState(IDLE);

	return ret;
}


