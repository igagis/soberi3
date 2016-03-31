/* 
 * File:   Application.hpp
 * Author: ivan
 *
 * Created on December 1, 2010, 12:05 AM
 */

#pragma once

#include <ting/Singleton.hpp>



class App : public ting::Singleton<App>{
public:
	App();

	~App();

	void MuteAudio();

	void UnmuteAudio();
};

