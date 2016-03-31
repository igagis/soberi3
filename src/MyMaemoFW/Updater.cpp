#include <ting/timer.hpp>
#include <ting/Ref.hpp>

#include "Updater.hpp"


Updater::Updater() :
		threadID(ting::Thread::GetCurrentThreadID()),
		oldTicks(0),
		isJustStarted(true)
{}



void Updater::Update(){
	if(!this->ThereIsSomethingToUpdate()){
		return;
	}

	ting::u32 dt;

	if(this->isJustStarted){
		this->isJustStarted = false;
		dt = 0;
		this->oldTicks = ting::timer::GetTicks();
	}else{
		ting::u32 ticks = ting::timer::GetTicks();
		dt = ticks - this->oldTicks;
		if(dt < 20){
			ting::Thread::Sleep(10);
			return;
		}
		this->oldTicks = ticks;
	}

	while(this->objToAdd.size() > 0){
		this->obj.push_back(this->objToAdd.back());
		this->objToAdd.pop_back();
	}

	for(TObjIter i = this->obj.begin(); i != this->obj.end();){
//		TRACE(<< "Update(): cycle" << std::endl)
		if(ting::Ref<Updateable> u = (*i)){
			if(u->isUpdating){
//				TRACE(<< "Update(): updating" << std::endl)
				u->Update(dt);
				++i;
			}else{
//				TRACE(<< "Update(): not updating" << std::endl)
				u->addedToUpdateList = false;
				
				i = this->obj.erase(i);
			}
		}else{
//			TRACE(<< "Update(): dead weak ref" << std::endl)
			i = this->obj.erase(i);
		}
	}

	if(this->obj.size() == 0){
		this->isJustStarted = true;
//		TRACE(<< "Updater::Update(): nothing to update" << std::endl)
	}
}


