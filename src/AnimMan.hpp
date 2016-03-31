// Author: Ivan Gagis <igagis@gmail.com>
// Created on October 28, 2008, 10:52 PM
// Version: 2

// Description:
//			Animation Manager class

#pragma once

#include <list>

#include <ting/Ptr.hpp>

#include <tride/Vector3.hpp>



//forward declaration
class AnimMan;



//base class for animations
class Animation{
	friend class AnimMan;
	
	ting::Inited<AnimMan*, 0> parent;
	
protected:
	inline AnimMan* Parent(){
		return this->parent;
	}

	Animation(tride::Vec2f pos = tride::Vec2f(0), float quat = 0) :
			time(0),
			p(pos),
			q(quat)
	{
		ASSERT(this->parent == 0)
	}

	float totalTime;
	float time;

public:
	tride::Vec2f p;//position
	float q;//rotation
	
	virtual ~Animation(){}//this is class is intended to be used as base class, need virtual destructor

	virtual void Render(const tride::Matr4f& matrix)const = 0;

	virtual bool Update(float dt){
		this->time += dt;

		if(this->time > this->totalTime){
			return true;
		}

		return false;
	}
};



class AnimMan : public Animation{
	typedef std::list<ting::Ptr<Animation> > T_AnimList;
	typedef T_AnimList::iterator T_AnimIter;
	typedef T_AnimList::const_iterator T_AnimConstIter;

	T_AnimList anim;

	T_AnimList toAdd;
public:

	AnimMan(){}

	~AnimMan(){}

	inline unsigned Size()const{
		return this->anim.size();
	}
private:
	//copy constructor
	AnimMan(const AnimMan& orig){}

public:

	//override
	void Render(const tride::Matr4f& matrix)const{
		tride::Matr4f matr(matrix);
		matr.Translate(this->p);
		//TODO: rotate by this->q

		for(T_AnimConstIter i = this->anim.begin(); i != this->anim.end(); ++i){
			ASSERT((*i).IsValid())
			(*i)->Render(matr);
		}
	}

	//override
	bool Update(float dt){
		while(this->toAdd.size() != 0){
			this->anim.push_back(this->toAdd.back());
			this->toAdd.pop_back();
		}

		for(T_AnimIter i = this->anim.begin(); i != this->anim.end(); ){
			ASSERT((*i).IsValid())
			if((*i)->Update(dt)){
				i = this->anim.erase(i);
			}else{
				++i;
			}
		}
		return this->anim.size() == 0;
	}

	inline void AddAnim(ting::Ptr<Animation> a){
		ASSERT(a)
		ASSERT(a->Parent() == 0)
		ASS(a)->parent = this;
		this->anim.push_back(a);
	}

	inline void Clear(){
		this->anim.clear();
	}
};//~class AnimMan

