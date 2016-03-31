#pragma once

#include <string>

#include <ting/Singleton.hpp>
#include <ting/Buffer.hpp>
#include <ting/types.hpp>

#include "Item.hpp"



class Preferences : public ting::Singleton<Preferences>{
	std::string prefsFileName;

public:
	Preferences(const std::string& fileName) :
			prefsFileName(fileName)
	{}

	void LoadFromFile();
	void SaveToFile();



	//popped items stats
private:
	ting::StaticBuffer<ting::Inited<unsigned, 0>, PopItem::NUM_ITEM_TYPES> numPoppedItems;

public:
	inline unsigned GetNumPoppedItems(PopItem::E_Type type)const{
		return this->numPoppedItems[ASSCOND(unsigned(type), < this->numPoppedItems.Size())];
	}

	inline void AddPoppedItem(PopItem::E_Type type){
		++(this->numPoppedItems[ASSCOND(unsigned(type), < this->numPoppedItems.Size())]);
	}



	//lightning, chainlocks and ice cubes stats
private:
	ting::Inited<unsigned, 0> numLightnings;
	ting::Inited<unsigned, 0> numChainLocks;
	ting::Inited<unsigned, 0> numIceCubes;

public:
	inline unsigned GetNumLightnings()const{
		return this->numLightnings;
	}

	inline void AddLightning(){
		++(this->numLightnings);
	}

	inline unsigned GetNumChainLocks()const{
		return this->numChainLocks;
	}

	inline void AddChainLock(){
		++(this->numChainLocks);
	}

	inline unsigned GetNumIceCubes()const{
		return this->numIceCubes;
	}

	inline void AddIceCube(){
		++(this->numIceCubes);
	}


private:
	ting::Inited<ting::u64, 0> timeSpentInGame;
public:
	inline ting::u64 GetTimeSpentInGame()const{
		return this->timeSpentInGame;
	}

	inline void AddTimeSpentInGame(ting::u32 dt){
		this->timeSpentInGame += dt;
	}




	//Audio settings
private:
	ting::Inited<bool, false> isSoundMuted;
	ting::Inited<bool, false> isMusicMuted;
public:
	inline void SetSoundUnmuted(bool unmuted){
		this->isSoundMuted = !unmuted;
	}

	inline bool IsSoundMuted()const{
		return this->isSoundMuted;
	}

	inline void SetMusicUnmuted(bool unmuted){
		this->isMusicMuted = !unmuted;
	}

	inline bool IsMusicMuted()const{
		return this->isMusicMuted;
	}
};
