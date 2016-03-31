/* The MIT License:

Copyright (c) 2009-2010 Ivan Gagis

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. */

#pragma once

#include <string>

#include <libosso.h>

#include <ting/Singleton.hpp>
#include <ting/Signal.hpp>



class LibOSSO : public ting::Singleton<LibOSSO>{
	osso_context_t* ossoContext;
	
public:

	ting::Signal0 wentToSleep;

	LibOSSO(const std::string& appName, const std::string& appVer){
		this->ossoContext = osso_initialize(
				appName.c_str(),
				appVer.c_str(),
				TRUE /* deprecated parameter */,
				0 /* Use default Glib main loop context */
			);

		if(!this->ossoContext){
			TRACE(<< "LibOSSO::LibOSSO(): osso_initialize() failed." << std::endl)
			throw ting::Exc("LibOSSO::LibOSSO(): osso_initialize() failed.");
		}

		//Set hardware messages handler
		{
			osso_return_t res = osso_hw_set_event_cb(
					this->ossoContext,
					NULL,
					&LibOSSO::DBusHWMessagesHandler,
					reinterpret_cast<gpointer>(this)
				);

			if(res != OSSO_OK){
				throw ting::Exc("LibOSSO::LibOSSO(): osso_hw_set_event_cb() failed.");
			}
		}

	}

	
	~LibOSSO(){
		ASSERT(this->ossoContext)
		osso_deinitialize(this->ossoContext);
	}


	void StartBrowser(const std::string& url){
		osso_return_t ret = osso_rpc_run_with_defaults(
				this->ossoContext,
				"osso_browser",
				"open_new_window",
				NULL,
				DBUS_TYPE_STRING,
				url.c_str(),
				DBUS_TYPE_INVALID
			);

		if(ret != OSSO_OK){
			throw ting::Exc("Launching browser failed");
		}
	}

private:
	static void DBusHWMessagesHandler(osso_hw_state_t *state, gpointer data){
		LibOSSO& libosso = *reinterpret_cast<LibOSSO*>(data);

		if(state->shutdown_ind){
			TRACE(<< "LibOSSO::DBusHWMessagesHandler(): Shutdown event" << std::endl)
		}

		if(state->memory_low_ind){
			TRACE(<< "LibOSSO::DBusHWMessagesHandler(): Memory low event" << std::endl)
		}

		if(state->save_unsaved_data_ind){
			TRACE(<< "LibOSSO::DBusHWMessagesHandler(): Must save unsaved data event" << std::endl)
		}

		if(state->system_inactivity_ind){
			TRACE(<< "LibOSSO::DBusHWMessagesHandler(): System went inactive event" << std::endl)
			libosso.wentToSleep.Emit();
		}
	}
};
