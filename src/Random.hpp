#pragma once


#include <ting/types.hpp>
#include <ting/timer.hpp>
#include <ting/Singleton.hpp>



class Random{
	ting::u32 a;

	static const ting::u32 maxLimit = 2796203;
	
public:
	Random(){
		this->a = ting::timer::GetTicks();
	}

	/**
	 * @brief Returns random number.
	 * @param limit - upper limit of rundom number.
	 * @return random number from [0:limit).
	 */
	inline ting::u32 Rand(ting::u32 limit){
		ASSERT(limit < maxLimit)
		this->a = (this->a * 125) % maxLimit;
		return ASSCOND(this->a % limit, < limit);
	}

	/**
	 * @brief Returns random number.
	 * @return random number from [0:1].
	 */
	inline float RandFloatZeroOne(){
		float ret = float(this->Rand(maxLimit - 1) + 1) / float(maxLimit - 1);
		ASSERT(0 <= ret && ret <= 1)
		return ret;
	}

	/**
	 * @brief Returns random number.
	 * @return random number from [-1:1].
	 */
	inline float RandFloatMinusOne_One(){
		float ret = this->RandFloatZeroOne() * 2 - 1;
		ASSERT(-1 <= ret && ret <= 1)
		return ret;
	}

	static inline Random& StaticInstance(){
		static Random rnd;
		return rnd;
	}
};



class GlobalRandom : public ting::Singleton<GlobalRandom>{
	Random rnd;
public:
	Random& Rnd(){
		return this->rnd;
	}
};
