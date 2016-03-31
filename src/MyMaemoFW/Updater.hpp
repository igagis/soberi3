/* 
 * File:   Updater.hpp
 * Author: ivan
 *
 * Created on March 10, 2010, 9:53 PM
 */

#pragma once


#include <vector>

#include <ting/Thread.hpp>
#include <ting/debug.hpp>
#include <ting/PoolStored.hpp>
#include <ting/Singleton.hpp>
#include <ting/Ptr.hpp>
#include <ting/Ref.hpp>



class Updateable : virtual public ting::RefCounted{
	friend class Updater;
	
	ting::Inited<bool, false> isUpdating;
	
	ting::Inited<bool, false> addedToUpdateList;
	
public:
	Updateable(){}
	
	virtual bool Update(ting::u32 dt) = 0;

	inline void StartUpdating();

	inline void StopUpdating();

	inline bool IsUpdating()const{
		return this->isUpdating;
	}
};



class Updater : public ting::Singleton<Updater>{
	friend class Updateable;

	unsigned threadID;

	typedef std::vector<ting::WeakRef<Updateable> > T_ObjList;
	typedef T_ObjList::iterator TObjIter;
	T_ObjList obj;

	T_ObjList objToAdd;

	ting::u32 oldTicks;
	bool isJustStarted;

	void AddForUpdate(const ting::WeakRef<Updateable>& u){
		if(this->obj.size() == 0){
//			TRACE(<< "Updater::AddForUpdate(): timer started" << std::endl)
		}
		
		this->objToAdd.push_back(u);
	}

public:
	void Update();

	Updater();

	inline bool ThereIsSomethingToUpdate()const{
		return this->obj.size() != 0 || this->objToAdd.size() != 0;
	}
};



inline void Updateable::StartUpdating(){
	ASSERT(ting::Thread::GetCurrentThreadID() == Updater::Inst().threadID)
	if(this->isUpdating){
		return;
	}

	this->isUpdating = true;

	//it is possible that StopUpdating was called right before calling StartUpdate,
	//so isUpdating was false but the object is added to updater list, check this:
	if(this->addedToUpdateList){
		return;//already updating
	}
	
	this->addedToUpdateList = true;
	
	Updater::Inst().AddForUpdate(this);
}



inline void Updateable::StopUpdating(){
	ASSERT(ting::Thread::GetCurrentThreadID() == Updater::Inst().threadID)

	this->isUpdating = false;
}
