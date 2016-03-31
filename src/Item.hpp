#pragma once


#include <tride/Vector3.hpp>

#include "Resources/ResSprite.hpp"
#include "Resources/ResParticleSystem.hpp"

#include "AnimMan.hpp"


inline float DItemFallingTime(){
	return 0.25f;
}

inline float DItemDiagonalFallingTime(){
//	return 1.4f * DItemFallingTime();
	return DItemFallingTime();
}

inline float DItemSwappingTime(){
	return 0.3f;
}



class Cell;



class Item{
public:
	float actTime;
	float totalActTime;

	float vel;

	tride::Vec2f movingDelta;

public:
	enum E_Type{
		POPITEM,
		ICECUBE
	} type;

	enum E_State{
		//Note only idle substates should go before IDLE state
		IDLE_HINTING_1,
		IDLE_HINTING_2,

		IDLE,

		SPAWNING,
		POPPING,
		MOVING,
		STOPPING_1,
		STOPPING_2,
		SWAPPING,
		SWAPPING_BACK,
		FINISHED_SWAPPING,
		HIT_BY_LIGHTNING_1,
		HIT_BY_LIGHTNING_2
	} state;

	E_State hasJustStoppedAction;//used to detect when Item stopped moving/spawning

	Cell* swappingCell;

protected:
	Item(E_Type t);
public:

	virtual ~Item(){}

	void SetState(E_State state);

	bool Update(float dt);

public:
	virtual bool SpecialUpdate(float dt){
		return false;
	}

	virtual void Render(const tride::Matr4f& matrix)const = 0;
public:

	inline void SetActTime(float time){
		this->totalActTime = time;
		this->actTime = 0;
	}

	void StartSpawning();

	void StartPopping(float actionTime = 0.5f){
		ASSERT(this->IsIdle())
		this->SetActTime(actionTime);
		this->SetState(POPPING);
	}

	void StartMoving(float duration, tride::Vec2f delta){
		ASSERT(this->IsIdle())
		this->movingDelta = delta;
		this->totalActTime = duration;
		this->SetState(MOVING);
	}

	void StartStopping();

	void StartHinting(tride::Vec2i dir);

	void StartSwappingBack(Cell* from, Cell* to);

	void StartSwapping(Cell* from, Cell* to);

	void StartHitByLightning();

	void ClearActTimes(){
		this->actTime = 0;
		this->totalActTime = 1;
	}

	inline bool IsPopItem()const{
		return this->type == POPITEM;
	}

	inline bool IsIdle()const{
		return this->state <= IDLE;
	}

	inline bool IsSwapping()const{
		return this->state == SWAPPING;
	}

	inline bool IsMoving()const{
		return this->state == MOVING;
	}

	inline bool IsPopping()const{
		return this->state == POPPING;
	}

	inline bool IsMoveable()const{
		return this->type == POPITEM;
	}

	inline bool IsSwapable()const{
		return this->type == POPITEM;
	}
};//~class Item



class IceCubeItem : public Item{
	ting::Inited<float, 0> asteriskTime;
	ting::Ref<ResSprite> asteriskSpr;

	AnimMan anim;
public:
	ting::Ref<ResSprite> noCracksSpr;
	ting::Ref<ResSprite> lightCracksSpr;
	ting::Ref<ResSprite> heavyCracksSpr;

	enum E_Cracks{
		NO_CRACKS,
		LIGHT_CRACKS,
		HEAVY_CRACKS,
		DESTROYED
	} cracks;

	IceCubeItem();

	bool Hit();

	//override
	void Render(const tride::Matr4f& matrix)const;

	//override
	bool SpecialUpdate(float dt);
};



class PopItem : public Item{
	ting::Ref<ResSprite> sprite;
	ting::Ref<ResParticleSystem> popPar;
public:
	ting::Ref<ResSprite> Sprite(){
		return this->sprite;
	}

	ting::Ref<ResParticleSystem> PopPar(){
		return this->popPar;
	}

	enum E_Type{
		ZERO,
		ONE,
		TWO,
		THREE,
		FOUR,
		FIVE,
		SIX,
		NUM_ITEM_TYPES,
		SEVEN
	} type;

	bool isGoalItem;

	PopItem(PopItem::E_Type t = ZERO, bool isGoal = false);

	//override
	void Render(const tride::Matr4f& matrix)const;

	static ting::Ref<ResSprite> LoadSpriteForItem(E_Type type);
};//~class PopItem
