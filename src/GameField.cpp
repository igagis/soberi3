/*
 * Copyright Ivan Gagis, 2010
 */

#include <algorithm>

#include <pugixml/pugixml.hpp>

#include <ting/math.hpp>

#include "ResMan/ResMan.hpp"
#include "Resources/ResParticleSystem.hpp"

#include "GameField.hpp"
#include "MyMaemoFW/Shader.hpp"
#include "MyMaemoFW/GLWindow.hpp"
#include "AnimMan.hpp"
#include "Lightning.hpp"
#include "utils.hpp"
#include "ParticleSystem.hpp"
#include "Preferences.hpp"



using namespace ting;
using namespace ting::math;
using namespace tride;



static const float DHintingTime = 3;//seconds
static const float DTimeForPop = 1;//seconds



GameField::GameField(ting::fs::File& levelFile) :
		numItemTypes(PopItem::NUM_ITEM_TYPES),
		selectedCell(0),
		grabbedCell(0),
		selAnimTime(0),
		possibleMoveStart(-1, -1),
		possibleMoveDirection(0, 0),
		hintTime(0)
{
//	TRACE(<< "GameField::GameField(): enter" << std::endl)

	this->progressBar = ProgressBarWidget::New("spr_progressbar", "spr_progressbar_frozen");
	this->Add(this->progressBar);

	//load POP sound effect
	this->popSnd = resman::ResMan::Inst().Load<ResSound>("snd_pop");

	//load SWAP sound effect
	this->swapSnd = resman::ResMan::Inst().Load<ResSound>("snd_swap");

	this->stopSnd = resman::ResMan::Inst().Load<ResSound>("snd_stop");

	this->iceCrackSnd = resman::ResMan::Inst().Load<ResSound>("snd_ice_crack");

	this->iceBreakSnd = resman::ResMan::Inst().Load<ResSound>("snd_ice_break");

	this->unlockingSnd = resman::ResMan::Inst().Load<ResSound>("snd_unlocking");

	this->selSpr = resman::ResMan::Inst().Load<ResSprite>("spr_selection");
	
	this->cellSpr = resman::ResMan::Inst().Load<ResSprite>("spr_grass");

	this->blockSpr = resman::ResMan::Inst().Load<ResSprite>("spr_rock");

	this->thinIceCellSpr = resman::ResMan::Inst().Load<ResSprite>("spr_thin_ice_cell");

	this->itemLockSpr = resman::ResMan::Inst().Load<ResSprite>("spr_item_lock");

	this->horiHollowBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_hori_barrier_hollow");

	this->horiSolidBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_hori_barrier_solid");

	this->vertHollowBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_vert_barrier_hollow");

	this->vertSolidBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_vert_barrier_solid");

	this->cornerHollowBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_corner_barrier_hollow");

	this->cornerSolidBarrierSpr = resman::ResMan::Inst().Load<ResSprite>("spr_corner_barrier_solid");

	this->asteriskSpr = resman::ResMan::Inst().Load<ResSprite>("spr_asterisk");

	this->icecubeHitPar = resman::ResMan::Inst().Load<ResParticleSystem>("par_icecube_hit");

//	TRACE(<< "GameField::GameField(): loading level" << std::endl)

	this->Resize(Vec2f(6, 9) * Cell::size);

	//load the level
	this->LoadLevel(levelFile);

//	TRACE(<< "GameField::GameField(): going to unpause" << std::endl)
	this->Unpause();
//	TRACE(<< "GameField::GameField(): going to unpaused" << std::endl)



	//TODO:testing code, remove it later
//	Ptr<ParticleSystem> parsys = resman::ResMan::Inst().Load<ResParticleSystem>("par_item_fragment_fire")->CreateParticleSystem();
//	parsys->p = Vec2f(240, 240);
//	this->topAnimMan.AddAnim(parsys);
}



GameField::~GameField()throw(){
}



void GameField::Pause(){
	this->StopUpdating();
}



void GameField::Unpause(){
	this->StartUpdating();
}



void GameField::ParseCellRowString(const char* rowStr, unsigned rowIndex){
	for(unsigned i = 0; *rowStr != 0; ++i){
//		LOG(<<(*rowStr))
		Cell *c = &this->CellAt(Vec2i(i, rowIndex));

		switch(*rowStr){
			case '.':
				c->type = Cell::CELL;
				break;
			case '=': //ice
				c->type = Cell::CELL;
				c->specialItem = Cell::THIN_ICE;
				break;
			case 'L': //lock
				c->type = Cell::CELL;
				c->specialItem = Cell::LOCK;
				break;
			case 'g':
				if(this->isPuzzleMode)
					throw ting::Exc("GameField::ParseCellRowString(): level file is malformed, puzzle mode level can't have generator cells");
				
				c->type = Cell::GENERATOR;
				break;
			case 'o':
				c->type = Cell::BLOCK;
				break;
			default:
				ASSERT_INFO(false, "GameField::ParseCellRowString(): undefined cell type = '" << char(*rowStr) << "'")
				break;
		}
		ASSERT(rowIndex < this->cell.Size())
		ASSERT(i < this->cell[rowIndex].Size())
		
		c->parent = this;
		c->SetPos(Vec2f(
				float(i) * Cell::size,
				float((this->cell.Size() - 1) - rowIndex) * Cell::size
			));
		++rowStr;
	}//~for(i)
}



void GameField::ParseVerticalBarriersRowString(const char* rowStr, unsigned rowIndex){
	ASSERT(rowIndex < this->verticalBarriers.Size())
	for(
			unsigned i = 0;
			*rowStr != 0 && i < this->verticalBarriers[rowIndex].Size();
			++i
		)
	{
		Barrier::EType type;
		switch(*rowStr){
			case ' ':
				type = Barrier::NO_BARRIER;
				break;
			case ':':
				type = Barrier::HOLLOW;
				break;
			case '|':
				type = Barrier::SOLID;
				break;
			default:
				ASSERT(false)
				break;
		}
		ASSERT(i < this->verticalBarriers[rowIndex].Size())
		Barrier *b = &this->verticalBarriers[rowIndex][i];
		b->parent = this;
		b->type = type;
		++rowStr;
	}//~for(i)
}



void GameField::ParseCornerBarriersRowString(const char* rowStr, unsigned rowIndex){
	for(
			unsigned i = 0;
			*rowStr != 0 && i < this->cornerBarriers[rowIndex].Size();
			++i
		)
	{
		Barrier::EType type;
		switch(*rowStr){
			case ' ':
				type = Barrier::NO_BARRIER;
				break;
			case '+':
				type = Barrier::HOLLOW;
				break;
			case '*':
				type = Barrier::SOLID;
				break;
			default:
				ASSERT(false)
				break;
		}
		Barrier *b = &this->cornerBarriers[rowIndex][i];
		b->parent = this;
		b->type = type;
		++rowStr;
	}//~for(i)
}



void GameField::ParseItemsRowString(const char* rowStr, unsigned rowIndex){
	for(
			unsigned i = 0;
			*rowStr != 0 && i < this->cell[rowIndex].Size();
			++i
		)
	{
		Cell* c = &this->cell[rowIndex][i];
		switch(*rowStr){
			case ' ':
				break;
			case 'p':
				c->initialPopItem = true;
				break;
			case '0':
				c->item = Ptr<Item>(new PopItem(PopItem::ZERO));
				break;
			case '1':
				c->item = Ptr<Item>(new PopItem(PopItem::ONE));
				break;
			case '2':
				c->item = Ptr<Item>(new PopItem(PopItem::TWO));
				break;
			case '3':
				c->item = Ptr<Item>(new PopItem(PopItem::THREE));
				break;
			case '4':
				c->item = Ptr<Item>(new PopItem(PopItem::FOUR));
				break;
			case '5':
				c->item = Ptr<Item>(new PopItem(PopItem::FIVE));
				break;
			case '6':
				c->item = Ptr<Item>(new PopItem(PopItem::SIX));
				break;
			case '=':
				c->item = Ptr<Item>(new IceCubeItem());
				break;
			default:
				ASSERT_INFO(false, "Unknown item type encountered in level: " << (*rowStr))
				break;
		}
		++rowStr;
	}//~for(i)
}



void GameField::ParseHorizontalBarriersRowString(const char* rowStr, unsigned rowIndex){
	for(
			unsigned i = 0;
			*rowStr != 0 && i < this->horizontalBarriers[rowIndex].Size();
			++i
		)
	{
		Barrier::EType type;
		switch(*rowStr){
			case ' ':
				type = Barrier::NO_BARRIER;
				break;
			case '-':
				type = Barrier::HOLLOW;
				break;
			case '=':
				type = Barrier::SOLID;
				break;
			default:
				ASSERT(false)
				break;
		}
		Barrier *b = &this->horizontalBarriers[rowIndex][i];
		b->parent = this;
		b->type = type;
		++rowStr;
	}//~for(i)
}



void GameField::LoadLevel(ting::fs::File& levelFile){
	{
		Array<u8> wholeFile = levelFile.LoadWholeFileIntoMemory();
		//make it null-terminated
		Array<u8> contents(wholeFile.Size() + 1);
		memcpy(contents.Begin(), wholeFile.Begin(), wholeFile.Size());
		contents[contents.Size() - 1] = 0;

		if(!this->levelDoc.load(reinterpret_cast<char*>(contents.Begin()))){
			throw Exc("GameField::LoadLevel(): file loading failed");
		}
	}

	this->Init();

//	TRACE(<< "GameField::LoadLevel(): level loaded" << std::endl)
}

void GameField::Init(){
	pugi::xml_node level = this->levelDoc.child("level");
	if(level.empty()){
		throw Exc("GameField::LoadLevel(): no \"level\" tag found");
	}

	//load tile set
	{
		pugi::xml_attribute a = level.attribute("tileSet");
		if(!a.empty()){
			if(std::string(a.value()) == "ground"){
				this->cellSpr = resman::ResMan::Inst().Load<ResSprite>("spr_ground");
				this->blockSpr = resman::ResMan::Inst().Load<ResSprite>("spr_rock_on_ground");
			}
		}
	}

	//load item types number
	{
		pugi::xml_attribute a = level.attribute("items");
		if(!a.empty()){
			this->numItemTypes = std::min(a.as_uint(), unsigned(PopItem::NUM_ITEM_TYPES));
		}else{
			this->numItemTypes = PopItem::NUM_ITEM_TYPES;
		}
	}

	//load puzzle/action mode
	{
		pugi::xml_attribute a = level.attribute("puzzle");
		if(!a.empty()){
			this->isPuzzleMode = a.as_bool();
		}else{
			this->isPuzzleMode = false;
		}
	}

	//load time
	{
		pugi::xml_attribute a = level.attribute("time");
		if(!a.empty()){
			this->totalTime = a.as_float();
		}else{
			this->totalTime = 10;
		}
	}

	//count rows
	unsigned numRows = 0;
	for(pugi::xml_node row = level.child("r"); !row.empty(); row = row.next_sibling("r")){
		++numRows;
	}

	if(numRows == 0){
		throw ting::Exc("no rows found in level");
	}

	//init cell rows array
	this->cell.Init(numRows);

	//init vertical barrier rows array
	this->verticalBarriers.Init(numRows);

	ASSERT(numRows >= 1)
	
	//init horizontal barrier rows array
	this->horizontalBarriers.Init(numRows - 1);

	//init corner barrier rows array
	this->cornerBarriers.Init(numRows - 1);

	pugi::xml_node row = level.child("r");
	for(unsigned j = 0; !row.empty();){
//		LOG(<<"GameField::LoadLevel(): row #"<<j<<std::endl)

		const char *rowStr = row.child_value();

//		TRACE(<< "GameField::LoadLevel(): parsing str = " << rowStr << std::endl)

		if(std::string("r") == row.name()){
			//parse row
			ASSERT(j < this->cell.Size())
			if(this->cell[j].Size() != 0){//if this row is already filled, then fill the next one
				++j;
			}
			//allocate memory for cells row and for its barrier rows
			ASSERT(this->cell[j].Size() == 0)
			this->cell[j].Init(strlen(rowStr));
			ASSERT(this->cell[j].Size() > 0)
			ASSERT(j < this->verticalBarriers.Size())
			this->verticalBarriers[j].Init(this->cell[j].Size() - 1);
			if(j < this->cornerBarriers.Size()){
				ASSERT(j < this->cornerBarriers.Size())
				this->cornerBarriers[j].Init(this->cell[j].Size() - 1);
			}
			if(j < this->horizontalBarriers.Size()){
				ASSERT(j < this->horizontalBarriers.Size())
				this->horizontalBarriers[j].Init(this->cell[j].Size());
			}
			this->ParseCellRowString(rowStr, j);
		}else if(std::string("vb") == row.name()){
			this->ParseVerticalBarriersRowString(rowStr, j);
		}else if(std::string("cb") == row.name()){
			if(j < this->cornerBarriers.Size()){
				this->ParseCornerBarriersRowString(rowStr, j);
			}
		}else if(std::string("hb") == row.name()){
			if(j < this->horizontalBarriers.Size()){
				this->ParseHorizontalBarriersRowString(rowStr, j);
			}
		}else if(std::string("i") == row.name()){
			this->ParseItemsRowString(rowStr, j);
		}else{
			ASSERT(false)
		}
//		LOG(<<std::endl)
		
		row = row.next_sibling();
	}//~for(j)

	this->timeLeft = this->totalTime / 2;
	ASSERT(this->timeLeft > 0)
	this->UpdateProgressBar();

	this->PopulateWithInitialItems();

	this->topAnimMan.Clear();
	this->subItemAnimMan.Clear();
	
	if(this->isPuzzleMode){
		this->InitPuzzleMode();

		this->state = IN_PROGRESS;
		this->Enable();
	}else{//action mode
		this->state = STARTING;
		this->Disable();
		this->topAnimMan.AddAnim(GetReadyTextAnim::New(this->Dim()));
	}

	this->possibleMoveStart = Vec2i(-1, -1);
	this->possibleMoveDirection = Vec2i(0, 0);

	this->ClearSelection();
}//~Init()



void GameField::InitPuzzleMode(){
	ASS(this->progressBar)->SetCompleteness(0);

	this->numItemsInPuzzleMode = 0;

	//calculate items
	this->totalNumItemsInPuzzleMode = 0;
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			const Cell& c = this->cell[j][i];

			if(c.item){
				switch(c.item->type){
					case Item::POPITEM:
					case Item::ICECUBE:
						++this->totalNumItemsInPuzzleMode;
						break;
					default:
						break;
				}
			}

			switch(c.specialItem){
				case Cell::LOCK:
				case Cell::THIN_ICE:
					++this->totalNumItemsInPuzzleMode;
					break;
				default:
					break;
			}
		}//~for(i)
	}//~for(j)
}



void GameField::PopulateWithInitialItems(){
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			if(
					this->cell[j][i].type != Cell::CELL &&
					this->cell[j][i].type != Cell::GENERATOR
				)
			{
				continue;
			}

			if(!this->cell[j][i].initialPopItem){
				continue;
			}
			
			unsigned numAllowedTypes = this->numItemTypes;
			ASSERT(this->numItemTypes <= PopItem::NUM_ITEM_TYPES)
			bool itemTypeAllowed[PopItem::NUM_ITEM_TYPES];

			for(unsigned k = 0; k < this->numItemTypes; ++k)
				itemTypeAllowed[k] = true;
			
			PopItem::E_Type type = PopItem::ZERO;

			while(numAllowedTypes > 0){
				unsigned num = this->rnd.Rand(numAllowedTypes) + 1;
//				LOG(<<"num = "<<num<<std::endl)
				ASSERT(num >= 1 && num <= numAllowedTypes && numAllowedTypes <= this->numItemTypes)
				//find the type
				for(unsigned k = 0; k < this->numItemTypes; ++k){
					if(itemTypeAllowed[k])
						--num;
					if(num == 0){
						type = PopItem::E_Type(k);
						break;
					}
				}
				
				if(this->MakesMatch(Vec2i(i, j), type)){
					itemTypeAllowed[unsigned(type)] = false;
					--numAllowedTypes;
#ifdef DEBUG
					if(numAllowedTypes == 0){
						TRACE_AND_LOG(<<"GameField::PopulateWithInitialItems(): no item type which does not make match"<<std::endl)
						break;
					}
#endif
				}else{
					break;
				}
			}
			
			ASSERT(type < PopItem::NUM_ITEM_TYPES)
			
			this->cell[j][i].item = ting::Ptr<PopItem>(new PopItem(type, true));
		}//~for(i)
	}//~for(j)
}



//override
bool GameField::OnMouseClick(const tride::Vec2f& position, EMouseButton button, bool isDown){
	if(button != Widget::LEFT)
		return false;

//	TRACE(<< "GameField::OnMouseClick(): left mouse button clicked, position = " << position << std::endl)

	if(isDown){
		Cell *cell = this->FindHoveredCell(position);
		if(!cell){
//			TRACE(<< "GameField::OnMouseClick(): no cell clicked" << std::endl)
			this->ClearSelection();
			return false;
		}

		if(ASS(cell)->specialItem == Cell::LOCK){
			return true;
		}

//		TRACE(<< "GameField::OnMouseClick(): clicked cell pos = " << cell->pos << std::endl)

		if(ASS(cell) == this->selectedCell){
//			TRACE(<<"GameField::OnLeftMouseClick(): drop selection"<<std::endl)
			this->ClearSelection();
		}else if(
				this->selectedCell &&
				ASS(cell)->item.IsValid() &&
				ASS(cell->item)->IsPopItem() &&
				ASS(cell->item)->IsIdle()
			)
		{ // swap
			ASSERT(cell != this->selectedCell)
			if(!this->TrySwapItems(cell, this->selectedCell)){
				this->selectedCell = cell;
			}else{
				this->ClearSelection();
			}
		}else if(
				ASS(cell)->item.IsValid() &&
				ASS(cell->item)->IsPopItem() &&
				ASS(cell->item)->IsIdle()
			)
		{
			this->selectedCell = cell; // change selection

//This code is just for testing Lightning
//			ASSERT(cell->item)
//			ASSERT(cell->item->IsPopItem())
//			this->topAnimMan.AddAnim(
//					this->lightningFactory.CreateLightning(
//							this->Dim(),
//							cell->PosOfCenter(),
//							cell->item.StaticCast<PopItem>()->Sprite(),
//							this->rnd
//						)
//				);
//			ASS(this->selectedCell->item)->StartHitByLightning();

		}
		this->grabbedCell = this->selectedCell;;
	}else{//released
		this->grabbedCell = 0;
	}

	return true;
}



//override
bool GameField::OnMouseMove(const tride::Vec2f& oldPos, const tride::Vec2f& newPos, const tride::Vec2f& dpos){
//	TRACE(<< "GameField::OnMouseMove(): invoked, newPos = " << newPos << std::endl)

	if(this->grabbedCell){
		ASSERT(this->grabbedCell == this->selectedCell)
		Cell *draggedToCell = this->FindHoveredCell(newPos);
		if(
				draggedToCell &&
				ASS(draggedToCell)->specialItem != Cell::LOCK &&
				ASS(draggedToCell)->item.IsValid() &&
				ASS(draggedToCell->item)->IsPopItem() &&
				ASS(draggedToCell->item)->IsIdle()
			)
		{
			if(TrySwapItems(draggedToCell, this->grabbedCell)){
				this->ClearSelection();
			}
		}
		return true;
	}
	return false;
}



bool GameField::CanBeSwapped(const tride::Vec2i& c1, const tride::Vec2i& dir){
	ASSERT(ting::math::Abs(dir.x) + ting::math::Abs(dir.y) == 1)

	Cell &cell1 = this->CellAt(c1);
	Cell &cell2 = this->CellAt(c1 + dir);

	if(cell1.item.IsNotValid()){
		return false;
	}
	
	if(cell2.item.IsNotValid()){
		return false;
	}

	if(
			cell1.specialItem == Cell::LOCK ||
			cell2.specialItem == Cell::LOCK
		)
	{
		return false;
	}

	if(!cell1.item->IsSwapable() || !cell2.item->IsSwapable()){
		return false;
	}

	if(!cell1.item->IsIdle() || !cell2.item->IsIdle()){
		return false;
	}

	//check vertical or horizontal barriers
	{
		if(dir.x == 0){//need to check horizontal barrier
			ASSERT(math::Abs(dir.y) == 1)
			int bp = std::min(c1.y, c1.y + dir.y);//y barrier pos
			ASSERT(bp >= 0)
			ASSERT(unsigned(bp) < this->horizontalBarriers.Size())
			ASSERT(c1.x >= 0)
			ASSERT(unsigned(c1.x) < this->horizontalBarriers[bp].Size())
			if(this->horizontalBarriers[bp][c1.x].type != Barrier::NO_BARRIER)
				return false;
		}else{//otherwise need to check vertical barrier
			ASSERT(Abs(dir.x) == 1)
			int bp = std::min(c1.x, c1.x + dir.x);//x barrier pos
			ASSERT(c1.y >= 0)
			ASSERT(unsigned(c1.y) < this->verticalBarriers.Size())
			ASSERT(bp >= 0)
			ASSERT(unsigned(bp) < this->verticalBarriers[c1.y].Size())
			if(this->verticalBarriers[c1.y][bp].type != Barrier::NO_BARRIER)
				return false;
		}
	}

	return true;
}



//returns true if items started swapping, false otherwise
bool GameField::TrySwapItems(Cell * cell1, Cell * cell2){
	Vec2i pos1 = this->GetCellIndices(cell1);
	Vec2i pos2 = this->GetCellIndices(cell2);
	ASSERT(pos1.x >= 0 && pos1.y >= 0)
	ASSERT(pos2.x >= 0 && pos2.y >= 0)
	Vec2i dir = pos2 - pos1;
	if(Abs(dir.x) + Abs(dir.y) != 1)
		return false;

	if(!this->CanBeSwapped(pos1, dir))
		return false;

	//initiate swapping process

	std::swap(cell1->item, cell2->item);
	
	cell1->item->StartSwapping(cell2, cell1);
	cell2->item->StartSwapping(cell1, cell2);

	ASSERT(cell1->item->swappingCell == cell2)
	ASSERT(cell2->item->swappingCell == cell1)

	//play swapping sound
	this->swapSnd->Play();

	return true;
}



//checks if diagonal slip is possible
bool GameField::CanSlipTo(Vec2i p){
	Cell *c = this->GetCell(p);
	if(!c)
		return false;
	if(c->type != Cell::CELL || !c->IsEmpty())
		return false;
	--p.y;

	//prevent diagonal slipping if vertical items can fall instead
	for(unsigned i = 0; i < 4; ++i){
		c = this->GetCell(p);
		if(!c)
			return true;
		if(c->type != Cell::CELL && c->type != Cell::GENERATOR)
			return true;
		if(c->specialItem == Cell::LOCK)
			return true;
		if(this->horizontalBarriers[p.y][p.x].type != Barrier::NO_BARRIER)
			return true;
		if(c->item.IsValid()){
			if(ASS(c->item)->IsPopItem()){
				return false;
			}else{
				return true;
			}
		}
		--p.y;
	}
	return true;
}



Vec2i GameField::GetCellIndices(Cell * c){
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			if (&this->CellAt(Vec2i(i, j)) == c)
				return Vec2i(i, j);
		}
	}
	return Vec2i(-1, -1);
}



Vec2i GameField::FindItemOwnerCellIndicies(const Item* item){
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			if (this->CellAt(Vec2i(i, j)).item == item)
				return Vec2i(i, j);
		}
	}
	return Vec2i(-1, -1);
}



//override
void GameField::Render(const tride::Matr4f& matrix)const{
//	TRACE(<< "GameField::Render(): invoked, matrix = " << matrix << std::endl)
//	TRACE(<< "GameField::Render(): this->Dim() = " << this->Dim() << std::endl)
	
	//render each cell
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			this->cell[j][i].Render(matrix);
		}
	}

	//render vertical barriers
	for(unsigned j = 0; j < this->verticalBarriers.Size(); ++j){
		for(unsigned i = 0; i < this->verticalBarriers[j].Size(); ++i){
			Matr4f m(matrix);
			m.Translate(this->cell[j][i].pos + Vec2f(Cell::size, float(Cell::size) / 2));
			this->verticalBarriers[j][i].RenderVertical(m);
		}
	}

	//render corner barriers
	for(unsigned j = 0; j < this->cornerBarriers.Size(); ++j){
		for(unsigned i = 0; i < this->cornerBarriers[j].Size(); ++i){
			Matr4f m(matrix);
			m.Translate(this->cell[j][i].pos + Vec2f(Cell::size, 0));
			this->cornerBarriers[j][i].RenderCorner(m);
		}
	}

	//render horizontal barriers
	for(unsigned j = 0; j < this->horizontalBarriers.Size(); ++j){
		for(unsigned i = 0; i < this->horizontalBarriers[j].Size(); ++i){
			Matr4f m(matrix);
			m.Translate(this->cell[j][i].pos + Vec2f(float(Cell::size) / 2, 0));
			this->horizontalBarriers[j][i].RenderHorizontal(m);
		}
	}

	this->subItemAnimMan.Render(matrix);

	//render items
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			this->cell[j][i].RenderItem(matrix);
		}
	}

	//render top level animations
	this->topAnimMan.Render(matrix);

	//render selection
	if(this->selectedCell){
//		TRACE(<< "GameField::Render(): selected cell pos = " << this->selectedCell->pos << std::endl)
		tride::Matr4f matr(matrix);
		matr.Translate(ASS(this->selectedCell)->PosOfCenter());
		matr.Scale(1.5f + 0.2f * sin(this->selAnimTime * 2 * DPi<float>()));
		matr.Scale(
				Cell::size / this->selSpr->Dim().x,
				Cell::size / this->selSpr->Dim().y
			);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		ASS(this->selSpr)->Render(matr);
	}
}



void GameField::UpdateProgressBar(){
	ASSERT(this->progressBar)
	float oldCompl = this->progressBar->Completeness();
	this->progressBar->SetCompleteness(this->timeLeft / ASSCOND(this->totalTime, > 0));

	const float DThr = 0.33;

	if(oldCompl > DThr && this->progressBar->Completeness() <= DThr){
		//launch hurry up text
		this->topAnimMan.AddAnim(Ptr<Animation>(
				new HurryUpTextAnim(this->Dim())
			));
	}
}



//override
bool GameField::Update(u32 deltaTime){
	Preferences::Inst().AddTimeSpentInGame(deltaTime);

	//mark window for redrawing in advance
	GLWindow::Inst().SetDirty();

	float dt = float(deltaTime) / 1000.0f;

	this->topAnimMan.Update(dt);
	this->subItemAnimMan.Update(dt);

	switch(this->state){
		case STARTING:
			if(this->topAnimMan.Size() == 0){//if "Get Ready! Go!" animation has gone
				this->state = IN_PROGRESS;
				this->Enable();
			}
			return false;
			break;
		case IN_PROGRESS:
			if(!this->isPuzzleMode){
				this->timeLeft -= dt;
				if(this->timeLeft < 0){
					this->timeLeft = 0;
					this->UpdateProgressBar();

					this->state = FAILED;
					this->Disable();
					this->ClearSelection();
					this->topAnimMan.AddAnim(LevelFailed::New(this->Dim()));
					return false;
				}
				this->UpdateProgressBar();
			}
			
			if(this->LevelIsCompleted()){
				TRACE(<< "Level completed!!!!!!!!!!!!!!!!!!!!!" << std::endl)
				this->state = COMPLETED;
				this->Disable();
				this->ClearSelection();
				this->topAnimMan.AddAnim(LevelCompleted::New(this->Dim()));
			}
			break;
		case COMPLETED:
			break;
		case FAILED:
			if(this->topAnimMan.Size() == 0 && this->subItemAnimMan.Size() == 0){
				this->Pause();
				this->levelFailed.Emit();
			}
			return false;
			break;
	}

	this->hintTime += dt;

	//update selection animation
	{
		this->selAnimTime += dt;
		if(this->selectedCell){
			if(this->selectedCell->item.IsNotValid() || !this->selectedCell->item->IsIdle()){
				this->ClearSelection();
			}
		}
	}
	
	//update each cell
	ASSERT(this->cell.Size() > 0)
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			this->cell[j][i].Update(dt);
		}
	}
	
	this->HandleItemsMovement();
	
	this->HandleItemsSwapping();
	
	if(this->PopMatchedItems()){
		//if all items are idle

		if(this->state == COMPLETED){
			if(this->topAnimMan.Size() == 0 && this->subItemAnimMan.Size() == 0){
				this->Pause();
				this->levelCompleted.Emit();
			}
		}else{
			if(this->possibleMoveStart.x < 0){
				this->UpdatePossibleMove();
	//			TRACE(<< "GameField::Update(): possible move = " << this->possibleMoveStart << this->possibleMoveDirection << std::endl)
				if(this->possibleMoveStart.x < 0){
					if(this->isPuzzleMode){
						//TODO: show "no more moves" dialog?
					}else{//in action mode
						this->KillRandomItem();
					}
				}
			}else{
				//There is possible move, check if hinting is needed
				if(this->hintTime >= DHintingTime){
					this->hintTime -= DHintingTime;
					if(!this->isPuzzleMode){
						this->StartHinting();
					}
				}
			}
		}
	}else{
		this->possibleMoveStart = Vec2i(-1, -1);
		this->hintTime = 0;
	}
	
	return false;
}



Barrier::EType GameField::GetLeftCornerBarrierType(tride::Vec2i c){
	ASSERT(c.y >= 0)
	ASSERT(c.x >= 0)
	if(unsigned(c.y) < this->cornerBarriers.Size()){
		if(
				c.x >= 1 &&
				unsigned(c.x) < this->cornerBarriers[c.y].Size() + 1
			)
		{
			return this->cornerBarriers[c.y][c.x - 1].type;
		}
	}
	return Barrier::NO_BARRIER;
}



Barrier::EType GameField::GetRightCornerBarrierType(tride::Vec2i c){
	ASSERT(c.y >= 0)
	ASSERT(c.x >= 0)
	if(unsigned(c.y) < this->cornerBarriers.Size()){
		if(
				unsigned(c.x) < this->cornerBarriers[c.y].Size()
			)
		{
			return this->cornerBarriers[c.y][c.x].type;
		}
	}
	return Barrier::NO_BARRIER;
}



void GameField::HandleItemsMovement(){
	bool someItemsHasStoppedMoving = false;

	for(int j = int(this->cell.Size()) - 1; j >= 0; --j){//from bottom to top
		for(int i = 0; unsigned(i) < this->cell[j].Size(); ++i){//from left to right
			Cell *c = &this->CellAt(Vec2i(i, j));
			if(c->item.IsNotValid())
				continue;

			if(!ASS(c->item)->IsMoveable())
				continue;

			if(c->specialItem == Cell::LOCK)
				continue;

			//check if item should jump to another cell
			if(!c->item->IsIdle())
				continue;

			Item* curItem = c->item.operator->();

			//check if the item can fall down
			Cell* cellBelow = this->GetCell(Vec2i(i, j) + Vec2i(0, 1));
			if(
					cellBelow && ASS(cellBelow)->IsEmpty() && //if there is a cell below and it is empty
					this->horizontalBarriers[j][i].type == Barrier::NO_BARRIER
				)
			{
				ASSERT(cellBelow->item.IsNotValid())
				cellBelow->item = c->item;
				cellBelow->item->StartMoving(DItemFallingTime(), c->pos - cellBelow->pos);
			}else{
				//Try diagonal slipping

				//Get left and right corner barrier types
				Barrier::EType leftbtype = this->GetLeftCornerBarrierType(Vec2i(i, j));
				Barrier::EType rightbtype = this->GetRightCornerBarrierType(Vec2i(i, j));

				//get random priority direction (left or right)
				int priorityDir = this->rnd.Rand(2);
				priorityDir = priorityDir * 2 - 1;
				ASSERT(Abs(priorityDir) == 1)

				//determine corner barrier in priority dir
				Barrier::EType pdbtype = priorityDir > 0 ? rightbtype : leftbtype;

				Cell *cSlipTo = 0;
				
				Vec2i slipTo = Vec2i(i, j) + Vec2i(priorityDir, 1);

				if(pdbtype == Barrier::NO_BARRIER && this->CanSlipTo(slipTo)){
					cSlipTo = &this->CellAt(slipTo);
				}else{
					//determine corner barrier in non-priority dir
					Barrier::EType npdbtype = (-priorityDir) > 0 ? rightbtype : leftbtype;

					slipTo = Vec2i(i, j) + Vec2i(-priorityDir, 1);
					if(npdbtype == Barrier::NO_BARRIER && this->CanSlipTo(slipTo)){
						cSlipTo = &this->CellAt(slipTo);
					}
				}

				if(cSlipTo){
					cSlipTo->item = c->item;
					cSlipTo->item->StartMoving(DItemDiagonalFallingTime(), c->pos - cSlipTo->pos);
				}
			}//~trying diagonal slipping

			//if item did not move away
			if(c->item){
				if(c->item->hasJustStoppedAction != Item::IDLE){
					if(c->item->IsIdle()){
						switch(c->item->hasJustStoppedAction){
							case Item::MOVING:
								someItemsHasStoppedMoving = true;

								//drop item velocity to 0, because it has stopped moving
								c->item->vel = 0;

//								c->item->actTime = 0;

								//start item stopping
								c->item->StartStopping();

								//crack ice cubes if any below us
								if(
										cellBelow &&
										ASS(cellBelow)->item.IsValid() &&
										ASS(cellBelow->item)->type == Item::ICECUBE
									)
								{
									if(cellBelow->item.StaticCast<IceCubeItem>()->Hit()){
										ting::Ptr<Animation> b = BreakIce(
												cellBelow->item.StaticCast<IceCubeItem>()->heavyCracksSpr,
												cellBelow->PosOfCenter(),
												this->rnd
											);
										this->topAnimMan.AddAnim(b);
										ASS(this->iceBreakSnd)->Play();

										if(this->isPuzzleMode){
											++this->numItemsInPuzzleMode;
											ASS(this->progressBar)->SetCompleteness(
													float(this->numItemsInPuzzleMode) / float(this->totalNumItemsInPuzzleMode)
												);
										}

										Preferences::Inst().AddIceCube();
									}else{
										ASS(this->iceCrackSnd)->Play();
									}

									Ptr<ParticleSystem> parsys = this->icecubeHitPar->CreateParticleSystem();
									parsys->p = cellBelow->PosOfCenter();
									this->topAnimMan.AddAnim(parsys);
								}
							case Item::SPAWNING:
								c->item->actTime = 0;
							default:
								break;
						}
					}
					c->item->hasJustStoppedAction = Item::IDLE;
				}
			}else{
				//no item in cell
				if(c->type == Cell::GENERATOR){
					c->GenerateItem();
					ASS(c->item)->actTime = ASS(curItem)->actTime;//synchronize
				}
			}
		}//~for(i)
	}//~for(j)

	if(someItemsHasStoppedMoving){
		ASS(this->stopSnd)->Play();
	}
}



void GameField::HandleItemsSwapping(){
	for(int j = int(this->cell.Size() - 1); j >= 0; --j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			Cell *c = &this->CellAt(Vec2i(i, j));
			if(c->item.IsValid()){//TODO: use continue
				//check if item has finished swapping and handle accordingly
				if(c->item->state == Item::FINISHED_SWAPPING){//TODO: use continue
					Cell *swc = c->item->swappingCell;
					ASSERT(swc)
					//IMPORTANT NOTE: (!!!)
					//While items are swapping none of them should disappear (e.g. bomb explode while swapping).
					//The bomb should not explode while it is in swapping state.
					ASSERT(swc->item)
					ASSERT(swc->item->state == Item::FINISHED_SWAPPING)
					ASSERT(swc->item->swappingCell == c)
					if(
								( c->item->type == Item::POPITEM &&
								this->MakesMatch(Vec2i(i, j), c->item.StaticCast<PopItem>()->type) )
								||
								( swc->item->type == Item::POPITEM &&
								this->MakesMatch(this->GetCellIndices(swc), swc->item.StaticCast<PopItem>()->type) )
							)
					{
						c->item->SetState(Item::IDLE);
						swc->item->SetState(Item::IDLE);
					}else{
						//clear swapping cells and set state to IDLE so we can start moving the items back then
						c->item->swappingCell = 0;
						c->item->SetState(Item::IDLE);
						swc->item->swappingCell = 0;
						swc->item->SetState(Item::IDLE);
						//move items back
						std::swap(c->item, swc->item);

						ASS(c->item)->StartSwappingBack(swc, c);
						ASS(swc->item)->StartSwappingBack(c, swc);

						//play swapping sound
						this->swapSnd->Play();
					}
				}//~if(c->item->state == FINISHED_SWAPPING)
			}//~if(c->item)
		}//~for(i)
	}//~for(j)
}



bool GameField::PopMatchedItems(){
	bool ret = true;
	
	//check for popping items
	typedef std::vector<Cell*> CellVec;
	typedef CellVec::iterator CellIter;
	CellVec cellsToPop;
	cellsToPop.reserve(100);
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			Vec2i p(i, j);
			Cell *c = &this->CellAt(p);
			if(c->item.IsValid()){
				if(c->item->IsIdle()){
					ASSERT(c->item->IsIdle())
					if(c->item->type == Item::POPITEM){
						if(this->MakesMatch(p, c->item.StaticCast<PopItem>()->type)){
							cellsToPop.push_back(c);
							ret = false;
						}
					}
				}else{
					ret = false;
				}
			}
		}//~for(i)
	}//~for(j)

	if(cellsToPop.size() == 0){
		return ret;
	}

	//pop matched items
	for(CellIter i = cellsToPop.begin(); i != cellsToPop.end(); ++i){
		Cell *c = (*i);
		ASSERT(c)
		ASSERT(c->item.IsValid())
		ASSERT(c->item->IsPopItem())
		c->item->StartPopping();

		Preferences::Inst().AddPoppedItem(c->item.StaticCast<PopItem>()->type);

		switch(c->specialItem){
			case Cell::THIN_ICE:
				{
					c->specialItem = Cell::NO_SPECIAL_ITEM;
					Ptr<Animation> b(new MeltingSnow(this->thinIceCellSpr, c->PosOfCenter()));
					this->subItemAnimMan.AddAnim(b);

					if(this->isPuzzleMode){
						++this->numItemsInPuzzleMode;
					}
				}
				break;
			case Cell::LOCK:
				c->specialItem = Cell::NO_SPECIAL_ITEM;
				//add unlocking anim
				this->topAnimMan.AddAnim(
						Ptr<Animation>(
								new UnlockedLock(this->itemLockSpr, c->PosOfCenter())
							)
					);
				this->unlockingSnd->Play();

				if(this->isPuzzleMode){
					++this->numItemsInPuzzleMode;
				}

				Preferences::Inst().AddChainLock();
				break;
			default:
				break;
		}

		//item pop particle system effect
		{
			ASSERT(c->item->IsPopItem())
			Ptr<ParticleSystem> parsys = c->item.StaticCast<PopItem>()->PopPar()->CreateParticleSystem();
			parsys->p = c->PosOfCenter();
			this->subItemAnimMan.AddAnim(parsys);
		}

		if(this->isPuzzleMode){
			++this->numItemsInPuzzleMode;
		}
	}

	ASSERT(cellsToPop.size() > 0)

	//add time for popped items
	this->AddSecondsToTime(cellsToPop.size() * DTimeForPop);

	//play sound
	this->popSnd->Play();

	if(this->isPuzzleMode){
		ASS(this->progressBar)->SetCompleteness(float(this->numItemsInPuzzleMode) / float(this->totalNumItemsInPuzzleMode));
	}

	return ret;
}



Cell * GameField::FindHoveredCell(const Vec2f& coord){
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			Cell *c = &this->CellAt(Vec2i(i, j));
			if(c->IsHovered(coord)){
				return c;
			}
		}	
	}
	return 0;
}



unsigned GameField::CalcMatchesAlongRay(Vec2i p, Vec2i dp, PopItem::E_Type value ) 	{
	ASSERT(
			(Abs(dp.x) == 1 && Abs(dp.y) == 0) ||
			(Abs(dp.x) == 0 && Abs(dp.y) == 1) ||
			(Abs(dp.x) == 1 && Abs(dp.y) == 1)
		)

	unsigned matchCount = 0;

	p += dp;
	
	while(this->CellIndIsValid(p) && 
				this->CellAt(p).item.IsValid() &&
				this->CellAt(p).item->IsIdle() &&
				this->CellAt(p).item->type == Item::POPITEM &&
				this->CellAt(p).item.StaticCast<PopItem>()->type == value
			)
	{
		//check barriers
		if(Abs(dp.x) == 0){ //if vertical direction
			Vec2i bp;
			ASSERT(Abs(dp.y) == 1)
			if(dp.y > 0){//direction down
				bp = p - dp;
			}else{//direction up
				bp = p;
			}
			ASSERT(bp.x >= 0 && bp.y >= 0)
			ASSERT_INFO(unsigned(bp.y) < this->horizontalBarriers.Size(), "bp.y = " << bp.y << " p.y = " << p.y << " dp.y = " << dp.y)
			ASSERT(unsigned(bp.x) < this->horizontalBarriers[bp.y].Size())
			if(this->horizontalBarriers[bp.y][bp.x].type == Barrier::SOLID){
				break;
			}
		}else if(Abs(dp.y) == 0){ //if horizontal direction
			Vec2i bp;
			ASSERT(Abs(dp.x) == 1)
			if(dp.x > 0){//direction right
				bp = p - dp;
			}else{//direction left
				bp = p;
			}
			if(this->verticalBarriers[bp.y][bp.x].type == Barrier::SOLID){
				break;
			}
		}else{ //diagonal direction
			Vec2i bp;
			ASSERT(Abs(dp.x) == 1 && Abs(dp.y) == 1)
			if(dp.x > 0){//direction right
				bp.x = p.x - dp.x;
			}else{//direction left
				bp.x = p.x;
			}
			if(dp.y > 0){//direction down
				bp.y = p.y - dp.y;
			}else{//direction up
				bp.y = p.y;
			}
			if(this->cornerBarriers[bp.y][bp.x].type == Barrier::SOLID){
				break;
			}
		}

		++matchCount;
		p += dp;
	}//~while()
	return matchCount;
}



bool GameField::MakesMatchInDir(Vec2i p, Vec2i dir, PopItem::E_Type value){
	if(this->CalcMatchesAlongRay(p, dir, value) +
				this->CalcMatchesAlongRay(p, -dir, value) + 1 >= 3
			)
		return true;
	return false;
}



//directions
//         -
//       /|\
//
static 	Vec2i dirs[] = {
	Vec2i(-1, -1),
	Vec2i( 0, -1),
	Vec2i( 1, -1),
	Vec2i( 1,  0)
};



bool GameField::MakesMatch(Vec2i p, PopItem::E_Type value){
	for(unsigned i = 0; i < sizeof(dirs)/sizeof(dirs[0]); ++i){
		if(this->MakesMatchInDir(p, dirs[i], value))
			return true;
	}
	return false;
}



void GameField::KillRandomItem(){
	Array<Array<bool> > cellsChecked(this->cell.Size());

	//init cellsChecked 2d array
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		cellsChecked[j].Init(this->cell[j].Size());
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			cellsChecked[j][i] = false;
		}
	}

	std::vector<Vec2i> cellsToCheck;

	//fill initial cells which are generator cells
	ASSERT(this->cell.Size() > 0)
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		ASSERT(this->cell[j].Size() > 0)
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			if(this->cell[j][i].type == Cell::GENERATOR){
				cellsToCheck.push_back(Vec2i(i, j));
			}
		}
	}

	std::vector<Vec2i> items;

	while(cellsToCheck.size() > 0){
		Vec2i c = cellsToCheck.back();
		cellsToCheck.pop_back();

		ASSERT(c.x >= 0)
		ASSERT(c.y >= 0)

		if(cellsChecked[c.y][c.x])
			continue;

		cellsChecked[c.y][c.x] = true;

		ASSERT_INFO(this->cell[c.y][c.x].item.IsValid(), "c = " << c)
		ASSERT(this->cell[c.y][c.x].item->type == Item::POPITEM)

		items.push_back(c);

		if(this->cell[c.y][c.x].specialItem == Cell::LOCK)
			continue;

		//check 3 possible directions of movement (down, down-left, down-right)

		ASSERT(this->cell.Size() > 1)
		if(unsigned(c.y) >= this->cell.Size() - 1)//if at the bottom
			continue;

		ASSERT(unsigned(c.y) < this->cell.Size() - 1)

		//down
		{
			Vec2i cellBelowIdx(c.x, c.y + 1);
			Cell* cellBelow = this->GetCell(cellBelowIdx);
			if(ASS(cellBelow)->type == Cell::CELL){
				if(this->horizontalBarriers[c.y][c.x].type == Barrier::NO_BARRIER){
					ASSERT_INFO(ASS(cellBelow)->item.IsValid(), "cellBelowIdx = " << cellBelowIdx)
					if(cellBelow->item->type == Item::POPITEM){
						cellsToCheck.push_back(cellBelowIdx);
					}
				}
			}
		}

		//down-left
		if(c.x > 0){
			Vec2i cellDLIdx(c.x - 1, c.y + 1);
			Cell* cellDL = this->GetCell(cellDLIdx);
			if(ASS(cellDL)->type == Cell::CELL){
				if(this->GetLeftCornerBarrierType(c) == Barrier::NO_BARRIER){
					ASSERT(ASS(cellDL)->item.IsValid())
					if(cellDL->item->type == Item::POPITEM){
						cellsToCheck.push_back(cellDLIdx);
					}
				}
			}
		}

		//down-right
		ASSERT(this->cell[c.y].Size() > 1)
		if(unsigned(c.x) < this->cell[c.y].Size() - 1){
			Vec2i cellDRIdx(c.x + 1, c.y + 1);
			Cell* cellDR = this->GetCell(cellDRIdx);
			if(ASS(cellDR)->type == Cell::CELL){
				if(this->GetRightCornerBarrierType(c) == Barrier::NO_BARRIER){
					ASSERT_INFO(ASS(cellDR)->item.IsValid(), "cellDRIdx = " << cellDRIdx)
					if(ASS(cellDR)->item->type == Item::POPITEM){
						cellsToCheck.push_back(cellDRIdx);
					}
				}
			}
		}
	}//~while(cellsToCheck.size() > 0)

	TRACE(<< "GameField::KillRandomItem(): found " << items.size() << " items reachable" << std::endl)

	ASSERT(items.size() > 0)

	{
		Cell* c = this->GetCell(items[this->rnd.Rand(items.size())]);
		ASSERT(c)

		ASSERT(c->item)
		ASSERT(c->item->IsPopItem())
		ASS(c->item)->StartHitByLightning();
		
		this->topAnimMan.AddAnim(
				this->lightningFactory.CreateLightning(
						this->Dim(),
						c->PosOfCenter(),
						c->item.StaticCast<PopItem>()->Sprite(),
						this->rnd
					)
			);
		this->lightningFactory.PlayThunderstrikeSound();
	}
	this->AddSecondsToTime(2 * DTimeForPop);

	Preferences::Inst().AddLightning();
}



void GameField::AddSecondsToTime(float seconds){
	this->timeLeft += seconds;
	ting::util::ClampTop(this->timeLeft, this->totalTime);
}



void GameField::UpdatePossibleMove(){
	//clear the possible move
	this->possibleMoveStart = Vec2i(-1, -1);

	ASSERT(this->cell.Size() > 0)
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		ASSERT(this->cell[j].Size() > 0)
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			Vec2i pos1(i, j);
			Cell &cell1 = this->CellAt(pos1);

			//check horizontal swap
			if(i < (this->cell[j].Size() - 1) && this->CanBeSwapped(pos1, Vec2i(1, 0))){
				Vec2i pos2 = pos1 + Vec2i(1, 0);
				Cell &cell2 = this->CellAt(pos2);
				
				//swap, check for matches, swap back
				std::swap(ASS(cell1.item), ASS(cell2.item));
				ASSERT(cell1.item->IsSwapable() && cell2.item->IsSwapable())
				ASSERT_INFO(cell1.item->IsPopItem(), "type is " << cell1.item->type)
				ASSERT_INFO(cell2.item->IsPopItem(), "type is " << cell2.item->type)
				PopItem::E_Type type1 = cell1.item.StaticCast<PopItem>()->type;
				PopItem::E_Type type2 = cell2.item.StaticCast<PopItem>()->type;
				if(this->MakesMatch(pos1, type1)){
					this->possibleMoveStart = pos1;
					this->possibleMoveDirection = pos2 - pos1;
				}else if(this->MakesMatch(pos2, type2)){
					this->possibleMoveStart = pos2;
					this->possibleMoveDirection = pos1 - pos2;
				}
				std::swap(ASS(cell1.item), ASS(cell2.item));

				if(this->possibleMoveStart.x >= 0){
					ASSERT(Abs(this->possibleMoveDirection.x) + Abs(this->possibleMoveDirection.y) == 1)
					return;
				}
			}

			//check vertical swap
			if(j < (this->cell.Size() - 1) && this->CanBeSwapped(pos1, Vec2i(0, 1))){
				Vec2i pos2 = pos1 + Vec2i(0, 1);
				Cell &cell2 = this->CellAt(pos2);

				//swap, check for matches, swap back
				std::swap(ASS(cell1.item), ASS(cell2.item));
				ASSERT(cell1.item->IsSwapable() && cell2.item->IsSwapable())
				ASSERT(cell1.item->IsPopItem())
				ASSERT(cell2.item->IsPopItem())
				PopItem::E_Type type1 = cell1.item.StaticCast<PopItem>()->type;
				PopItem::E_Type type2 = cell2.item.StaticCast<PopItem>()->type;
				if(this->MakesMatch(pos1, type1)){
					this->possibleMoveStart = pos1;
					this->possibleMoveDirection = pos2 - pos1;
				}else if(this->MakesMatch(pos2, type2)){
					this->possibleMoveStart = pos2;
					this->possibleMoveDirection = pos1 - pos2;
				}
				std::swap(ASS(cell1.item), ASS(cell2.item));

				if(this->possibleMoveStart.x >= 0){
					ASSERT(Abs(this->possibleMoveDirection.x) + Abs(this->possibleMoveDirection.y) == 1)
					return;
				}
			}
		}//~for(i)
	}//~for(j)
}



bool Cell::IsHovered(Vec2f point)const{
	return
			this->pos.x <= point.x &&
			this->pos.y <= point.y &&
			this->pos.x + this->size > point.x &&
			this->pos.y + this->size > point.y;
}



void Cell::Render(const tride::Matr4f& matrix)const{
//	TRACE(<< "Cell::Render(): invoked, matrix = " << matrix << std::endl)

	tride::Matr4f matr(matrix);
	matr.Translate(this->pos);
//	matr.Scale(Cell::size);

//	TRACE(<< "Cell::Render(): p = " << this->pos << std::endl)
//	TRACE(<< "Cell::Render(): matr = " << matr << std::endl)

	//render cell
	switch(this->type){
		case BLOCK:
			ASS(ASS(this->parent)->blockSpr)->Render(matr);
			break;
		case GENERATOR:
		case CELL:
			{
//				Matr4f m(matr);
//				m.Translate(Vec2f(0.5, 0.5));
//				m.Scale(0.45);
				
//				SimpleColoringShader &s = SimpleColoringShader::Inst();
//				s.UseProgram();
//				s.SetMatrix(m);
//				s.DisableColorPointer();
//				s.SetColor(tride::Vec3f(0.5, 0.5, 0.5));
//				s.DrawQuad(GL_TRIANGLES);


//				switch(this->specialItem){
//					default:
//						ASSERT(false)
//					case Cell::LOCK:
//					case Cell::NO_SPECIAL_ITEM:
//						ASS(ASS(this->parent)->cellSpr)->Render(matr);
//						break;
//					case Cell::THIN_ICE:
//						ASS(ASS(this->parent)->thinIceCellSpr)->Render(matr);
//						break;
//				}
				glDisable(GL_BLEND);//to make it render faster
				ASS(ASS(this->parent)->cellSpr)->Render(matr);
				if(this->specialItem == Cell::THIN_ICE){
					Matr4f m(matr);
					m.Translate(Vec2f(Cell::size, Cell::size) / 2);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glEnable(GL_BLEND);
					ASS(ASS(this->parent)->thinIceCellSpr)->Render(m);
				}
			}
			break;
		default:
			break;
	}
}



void Cell::RenderItem(const tride::Matr4f& matrix)const{
	tride::Matr4f matr(matrix);
	matr.Translate(this->pos + Vec2f(Cell::size) / 2);

	//render item
	if(this->item.IsValid()){
		this->item->Render(matr);
	}

	if(this->specialItem == Cell::LOCK){
		//render lock
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		ASS(ASS(this->parent)->itemLockSpr)->Render(matr);
	}
}



void Cell::GenerateItem(){
	ASSERT(this->type == GENERATOR)
	ASSERT(this->item.IsNotValid())

	ASSERT(ASS(this->parent)->NumItemTypes() <= PopItem::NUM_ITEM_TYPES)

	int randomType = ASS(this->parent)->rnd.Rand(ASS(this->parent)->NumItemTypes());
	this->item = ting::Ptr<PopItem>(
			new PopItem(PopItem::E_Type(randomType))
		);
	this->item->StartSpawning();
}



bool Cell::Update(float dt){
	//update item
	if(this->item.IsValid()){
		if(this->item->Update(dt))//item popped?
			this->item.Reset();
	}

	//update special item
	switch(this->specialItem){
		case LOCK:
			{
				const float DAsteriskTimeStep = 0.2;//0.2 second
				const float DProbabilityPerSecond = 0.2;

				this->asteriskTime += dt;
				while(this->asteriskTime > 0){
					this->asteriskTime -= DAsteriskTimeStep;
					if(
							ASS(this->parent)->rnd.RandFloatZeroOne()
									< (1 - ting::math::Exp(DAsteriskTimeStep * ting::math::Ln(1 - DProbabilityPerSecond)))
						)
					{
						this->parent->topAnimMan.AddAnim(Ptr<Animation>(
								new Asterisk(parent->asteriskSpr, this->PosOfCenter() + Vec2f(25, 25))
							));
					}
				}
			}
			break;
		default:
			break;
	}

	//generate item after item updating, to avoid empty generator cells.
	if(this->type == GENERATOR){
		if(this->item.IsNotValid()){
//			float randomVal = ASS(this->parent)->rnd.Rand(2);
//			if(randomVal / dt < 0.3){
			this->GenerateItem();
//			}
/*
			else{
				float randomVal = ASS(this->parent)->rnd.Rand(2);
				if(randomVal / dt < 0.01){
					this->item = ting::Ptr<BombItem>(
							new BombItem()
						);
					this->item->StartSpawning();
				}
			}
*/
		}
	}

	return false;
}



const float barrierHalfThickness = 6;


void Barrier::RenderVertical(const tride::Matr4f& matrix)const{
	if(this->type == Barrier::NO_BARRIER)
		return;

//	SimpleColoringShader &s = SimpleColoringShader::Inst();
//	s.UseProgram();
//
//	Matr4f matr(matrix);
//	matr.Scale(Vec2f(barrierHalfThickness, float(Cell::size) / 4));
//
//	s.SetMatrix(matr);
//	s.SetColor(Vec3f(1, 1, 1));

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if(this->type == Barrier::SOLID){
//		s.DrawQuad(GL_TRIANGLES);
		ASS(this->parent)->vertSolidBarrierSpr->Render(matrix);
	}else{
		ASSERT(this->type == Barrier::HOLLOW)
		ASS(this->parent)->vertHollowBarrierSpr->Render(matrix);
//		s.DrawQuad(GL_LINE_LOOP);
	}
}



void Barrier::RenderCorner(const tride::Matr4f& matrix)const{
	if(this->type == Barrier::NO_BARRIER)
		return;

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if(this->type == Barrier::SOLID){
//		s.DrawQuad(GL_TRIANGLES);
		ASS(this->parent)->cornerSolidBarrierSpr->Render(matrix);
	}else{
		ASSERT(this->type == Barrier::HOLLOW)
		ASS(this->parent)->cornerHollowBarrierSpr->Render(matrix);
//		s.DrawQuad(GL_LINE_LOOP);
	}

//	SimpleColoringShader &s = SimpleColoringShader::Inst();
//	s.UseProgram();
//
//	s.SetColor(Vec3f(1, 1, 1));

//	{
//		Matr4f matr(matrix);
//		matr.Scale(Vec2f(barrierHalfThickness, float(Cell::size) / 4));
//
//		s.SetMatrix(matr);
//
//		if(this->type == Barrier::SOLID){
//			s.DrawQuad(GL_TRIANGLES);
//		}else{
//			ASSERT(this->type == Barrier::HOLLOW)
//			s.DrawQuad(GL_LINE_LOOP);
//		}
//	}
//
//	{
//		Matr4f matr(matrix);
//		matr.Scale(Vec2f(float(Cell::size) / 4, barrierHalfThickness));
//
//		s.SetMatrix(matr);
//
//		if(this->type == Barrier::SOLID){
//			s.DrawQuad(GL_TRIANGLES);
//		}else{
//			ASSERT(this->type == Barrier::HOLLOW)
//			s.DrawQuad(GL_LINE_LOOP);
//		}
//	}
}



void Barrier::RenderHorizontal(const tride::Matr4f& matrix)const{
	if(this->type == Barrier::NO_BARRIER)
		return;

//	SimpleColoringShader &s = SimpleColoringShader::Inst();
//	s.UseProgram();

//	Matr4f matr(matrix);
//	matr.Scale(Vec2f(float(Cell::size) / 4, barrierHalfThickness));

//	s.SetMatrix(matr);
//	s.SetColor(Vec3f(1, 1, 1));

	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	if(this->type == Barrier::SOLID){
//		s.DrawQuad(GL_TRIANGLES);
		ASS(this->parent)->horiSolidBarrierSpr->Render(matrix);
	}else{
		ASSERT(this->type == Barrier::HOLLOW)
		ASS(this->parent)->horiHollowBarrierSpr->Render(matrix);
//		s.DrawQuad(GL_LINE_LOOP);
	}
}



unsigned GameField::LevelIsCompleted(){
	for(unsigned j = 0; j < this->cell.Size(); ++j){
		for(unsigned i = 0; i < this->cell[j].Size(); ++i){
			Cell *c = this->GetCell(Vec2i(i, j));
			if(c->specialItem != Cell::NO_SPECIAL_ITEM)
				return false;

			if(c->item.IsValid()){
				if(ASS(c->item)->type == Item::ICECUBE)
					return false;

				if(this->isPuzzleMode){
					if(ASS(c->item)->type == Item::POPITEM)
						return false;
				}
			}//~if(item is valid)
		}
	}

	return true;
}



void GameField::StartHinting(){
//	TRACE(<< "GameField::StartHinting(): invoked" << std::endl)
	ASSERT(this->possibleMoveStart.IsPositiveOrZero())
	ASSERT(Abs(this->possibleMoveDirection.x) <= 1 && Abs(this->possibleMoveDirection.y) <= 1)

	ASSERT(this->CellIndIsValid(this->possibleMoveStart));
	Cell *c1 = ASS(this->GetCell(this->possibleMoveStart));
	ASSERT(this->CellIndIsValid(this->possibleMoveStart + this->possibleMoveDirection));
	Cell *c2 = ASS(this->GetCell(this->possibleMoveStart + this->possibleMoveDirection));

	ASSERT(c1->item.IsValid())
	ASSERT(c1->item->IsPopItem())
	ASSERT(c2->item.IsValid())
	ASSERT(c2->item->IsPopItem())

	Vec2i movingDir(this->possibleMoveDirection.x, -this->possibleMoveDirection.y);
	c1->item->StartHinting(movingDir);
	c2->item->StartHinting(-movingDir);
}



//override
void GameField::OnResize(){
	ASS(this->progressBar)->Move(Vec2f(
			(this->Dim().x - this->progressBar->Dim().x) / 2,
			9 * Cell::size - 5
		));
}

