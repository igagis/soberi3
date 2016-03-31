#pragma once

#include <ting/types.hpp>
#include <ting/Ptr.hpp>
#include <ting/Array.hpp>
#include <ting/Singleton.hpp>
#include <ting/Signal.hpp>

#include <tride/Vector3.hpp>

#include "MyMaemoFW/Widget.hpp"
#include "MyMaemoFW/Updater.hpp"

#include "ResMan/ResMan.hpp"
#include "Resources/ResSound.hpp"
#include "Resources/ResParticleSystem.hpp"

#include "AnimMan.hpp"
#include "Random.hpp"
#include "Item.hpp"
#include "Lightning.hpp"
#include "ProgressBarWidget.hpp"



//forward declarations
class GameField;


class Cell{
	ting::Inited<float, 0> asteriskTime;
public:
	tride::Vec2f pos;

	inline tride::Vec2f PosOfCenter(){
		return this->pos + tride::Vec2f(size, size) / 2;
	}

	GameField* parent;
	
	ting::Ptr<Item> item;
	static const int size = 80;
	
	enum E_Type{
		BLOCK,
		CELL,
		GENERATOR
	} type;

	enum EIce{
		NO_SPECIAL_ITEM,
		THIN_ICE,
		LOCK
	} specialItem;

	bool initialPopItem;//if true, the initial pop item will be generated in this cell on level load

	Cell() :
			pos(0, 0),
			type(CELL),
			specialItem(NO_SPECIAL_ITEM),
			initialPopItem(false)
	{}
	
	inline void SetPos(tride::Vec2f p){
		this->pos = p;
	}

	void GenerateItem();
	
	bool IsHovered(tride::Vec2f point)const;
	
	inline bool IsEmpty()const{
		return this->type == CELL && this->item.IsNotValid();
	}
	
	bool Update(float dt);



	void Render(const tride::Matr4f& matrix)const;


	void RenderItem(const tride::Matr4f& matrix)const;
};//~class Cell



class Barrier{
public:
	enum EType{
		NO_BARRIER,
		HOLLOW,
		SOLID
	} type;

	GameField *parent;

	Barrier() :
			type(NO_BARRIER),
			parent(0)
	{}

	void RenderVertical(const tride::Matr4f& matrix)const;

	void RenderHorizontal(const tride::Matr4f& matrix)const;

	void RenderCorner(const tride::Matr4f& matrix)const;
};



class GameField : public Widget, public Updateable{
	//TODO: change to StaticBuffer<StaticBuffer<> >
	ting::Array<ting::Array<Cell> > cell;
	
	inline Cell& CellAt(const tride::Vec2i& p){
		ASSERT(this->CellIndIsValid(p))
		return this->cell[p.y][p.x];
	}

	unsigned numItemTypes;

public:
	inline unsigned NumItemTypes()const{
		return this->numItemTypes;
	}

private:
	ting::Array<ting::Array<Barrier> > verticalBarriers;
	ting::Array<ting::Array<Barrier> > horizontalBarriers;
	ting::Array<ting::Array<Barrier> > cornerBarriers;


	ting::Ref<ResSound> popSnd;
	ting::Ref<ResSound> swapSnd;
	ting::Ref<ResSound> stopSnd;
	ting::Ref<ResSound> iceCrackSnd;
	ting::Ref<ResSound> iceBreakSnd;
	ting::Ref<ResSound> unlockingSnd;

	GameField(ting::fs::File& levelFile);
	
public:
	ting::Ref<ResSprite> cellSpr;
	ting::Ref<ResSprite> blockSpr;
	ting::Ref<ResSprite> horiHollowBarrierSpr;
	ting::Ref<ResSprite> horiSolidBarrierSpr;
	ting::Ref<ResSprite> vertHollowBarrierSpr;
	ting::Ref<ResSprite> vertSolidBarrierSpr;
	ting::Ref<ResSprite> cornerHollowBarrierSpr;
	ting::Ref<ResSprite> cornerSolidBarrierSpr;

	ting::Ref<ResSprite> thinIceCellSpr;
	ting::Ref<ResSprite> itemLockSpr;

	ting::Ref<ResSprite> asteriskSpr;

	ting::Ref<ResParticleSystem> icecubeHitPar;

	LightningFactory lightningFactory;
	
	Random rnd;

	static ting::Ref<GameField> New(ting::fs::File& levelFile){
		return ting::Ref<GameField>(new GameField(levelFile));
	}

	AnimMan topAnimMan;
	AnimMan subItemAnimMan;

	~GameField()throw();

	ting::Signal0 levelCompleted;
	ting::Signal0 levelFailed;

private:
	Cell *selectedCell;
	Cell *grabbedCell;

	inline void ClearSelection(){
		this->selectedCell = 0;
		this->grabbedCell = 0;
	}

	ting::Ref<ResSprite> selSpr;
	float selAnimTime;

private:
	enum EState{
		STARTING,
		IN_PROGRESS,
		COMPLETED,
		FAILED
	} state;

	//If true, then the game is in puzzle mode, otherwise it is in action mode
	bool isPuzzleMode;
	unsigned totalNumItemsInPuzzleMode;
	unsigned numItemsInPuzzleMode;
	void InitPuzzleMode();
public:
	inline bool IsPuzzleLevel()const{
		return this->isPuzzleMode;
	}

private:
	ting::Ref<ProgressBarWidget> progressBar;
	float totalTime;
	float timeLeft;
	void AddSecondsToTime(float seconds);
	void UpdateProgressBar();
public:
	void Pause();
	void Unpause();
	bool IsPaused()const{
		return !this->IsUpdating();
	}

public:
	void Init();
private:
	pugi::xml_document levelDoc;

	void LoadLevel(ting::fs::File& levelFile);

	void ParseCellRowString(const char* rowStr, unsigned rowIndex);
	void ParseVerticalBarriersRowString(const char* rowStr, unsigned rowIndex);
	void ParseCornerBarriersRowString(const char* rowStr, unsigned rowIndex);
	void ParseHorizontalBarriersRowString(const char* rowStr, unsigned rowIndex);
	void ParseItemsRowString(const char* rowStr, unsigned rowIndex);

	void PopulateWithInitialItems();
public:

	//override
	virtual bool OnMouseClick(const tride::Vec2f& pos, EMouseButton button, bool isDown);

	//override
	virtual bool OnMouseMove(const tride::Vec2f& oldPos, const tride::Vec2f& newPos, const tride::Vec2f& dpos);

	
	inline bool CellIndIsValid(tride::Vec2i p){
		return p.y >= 0 && uint(p.y) < this->cell.Size() &&
					p.x >= 0 && uint(p.x) < this->cell[p.y].Size();
	}
	
	inline Cell* GetCell(tride::Vec2i p){
		if(this->CellIndIsValid(p))
			return &this->CellAt(p);
		return 0;
	}
	
	
	//returns true if cell items can be swapped
	bool CanBeSwapped(const tride::Vec2i& c1, const tride::Vec2i& dir);

	//Retruns true if items started swapping or false if items are not swappable
	bool TrySwapItems(Cell * cell1, Cell * cell2);
	
	tride::Vec2i GetCellIndices(Cell * c);
	
	tride::Vec2i FindItemOwnerCellIndicies(const Item* item);
	
	Cell * FindHoveredCell(const tride::Vec2f& coord);

	void HandleItemsMovement();
	void HandleItemsSwapping();

	//returns true if no items to pop and all items are in idle state
	bool PopMatchedItems();
	
	bool MakesMatch(tride::Vec2i p, PopItem::E_Type value);
	bool MakesMatchInDir(tride::Vec2i p, tride::Vec2i dir, PopItem::E_Type value);

	unsigned CalcMatchesAlongRay(tride::Vec2i p, tride::Vec2i dp, PopItem::E_Type value );

private:
	tride::Vec2i possibleMoveStart;//-1 means no possible move
	tride::Vec2i possibleMoveDirection;

	float hintTime;

	void StartHinting();

	void UpdatePossibleMove();

	void KillRandomItem();

	unsigned LevelIsCompleted();

	Barrier::EType GetLeftCornerBarrierType(tride::Vec2i c);
	Barrier::EType GetRightCornerBarrierType(tride::Vec2i c);
public:
	bool CanSlipTo(tride::Vec2i p);

	//override
	virtual bool Update(ting::u32 dt);
	
	//override
	void Render(const tride::Matr4f& matrix)const;

	//override
	void OnResize();
};//~class GameField

