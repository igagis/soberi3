#include <sstream>

#include <pugixml/pugixml.hpp>

#include <ting/fs/FSFile.hpp>

#include "Preferences.hpp"



using namespace ting;



const char* DStatisticsTag = "Statistics";
const char* DPopItemsTypePrefix = "popItemType";

const char* DNumLightningsAttr = "NumLightnings";
const char* DNumChainLocksAttr = "NumChainLocks";
const char* DNumIceCubesAttr = "NumIceCubes";

const char* DTimeSpentInGameLAttr = "TimeInGameL";
const char* DTimeSpentInGameHAttr = "TimeInGameH";


const char* DAudioTag = "Audio";
const char* DSoundMutedAttr = "SoundMuted";
const char* DMusicMutedAttr = "MusicMuted";



void Preferences::LoadFromFile(){
//	TRACE(<< "MainWindow::LoadPreferencesFromFile(): enter" << std::endl)

	ting::fs::FSFile f(this->prefsFileName);

//	TRACE(<< "MainWindow::LoadPreferencesFromFile(): reading file" << std::endl)

	pugi::xml_document doc;
	{
		ting::Array<ting::u8> rawFile;
		try{
			rawFile = f.LoadWholeFileIntoMemory();
		}catch(ting::fs::File::Exc &e){
			LOG(
					<< "Preferences::LoadFromFile(): could not load preferences file." << std::endl
					<< "\t" << " e = " << e.What() << std::endl
				)
			return;
		}

		doc.load_buffer(rawFile.Begin(), rawFile.Size());
	}



	//Load statistics
	{
		pugi::xml_node e = doc.child(DStatisticsTag);
		if(!e.empty()){
			//Load numbers of popped items
			for(unsigned i = 0; i < PopItem::NUM_ITEM_TYPES; ++i){
				std::stringstream itemTypeAttrName;
				itemTypeAttrName << DPopItemsTypePrefix << i;
				
				pugi::xml_attribute a = e.attribute(itemTypeAttrName.str().c_str());
				if(a.empty())
					continue;

				this->numPoppedItems[ASSCOND(i, < this->numPoppedItems.Size())] = a.as_uint();
			}

			//Load number of lightnings
			{
				pugi::xml_attribute a = e.attribute(DNumLightningsAttr);
				if(!a.empty()){
					this->numLightnings = a.as_uint();
				}
			}

			//Load number of chainlocks
			{
				pugi::xml_attribute a = e.attribute(DNumChainLocksAttr);
				if(!a.empty()){
					this->numChainLocks = a.as_uint();
				}
			}

			//Load number of ice cubes
			{
				pugi::xml_attribute a = e.attribute(DNumIceCubesAttr);
				if(!a.empty()){
					this->numIceCubes = a.as_uint();
				}
			}

			//Load time spent in game
			{
				pugi::xml_attribute a = e.attribute(DTimeSpentInGameHAttr);
				if(!a.empty()){
					this->timeSpentInGame = a.as_uint();
				}
				this->timeSpentInGame = (this->timeSpentInGame << 32);

				a = e.attribute(DTimeSpentInGameLAttr);
				if(!a.empty()){
					this->timeSpentInGame += a.as_uint();
				}
			}
		}
	}



	//Load audio settings
	{
		pugi::xml_node e = doc.child(DAudioTag);
		if(!e.empty()){
			//Load sound muted setting
			{
				pugi::xml_attribute a = e.attribute(DSoundMutedAttr);
				if(!a.empty()){
					this->SetSoundUnmuted(!a.as_bool());
				}
			}

			//Load music muted setting
			{
				pugi::xml_attribute a = e.attribute(DMusicMutedAttr);
				if(!a.empty()){
					this->SetMusicUnmuted(!a.as_bool());
				}
			}
		}
	}

	//TODO:
}



void Preferences::SaveToFile(){
	pugi::xml_document doc;

	//add xml version info
	{
		pugi::xml_node n = doc.append_child(pugi::node_declaration);
		n.set_name("xml");
		n.append_attribute("version").set_value("1.0");
	}




	//Save statistics
	{
		pugi::xml_node opts = doc.append_child();
		opts.set_name(DStatisticsTag);

		//Save Numers of popped items
		for(unsigned i = 0; i < this->numPoppedItems.Size(); ++i){
			std::stringstream itemTypeAttrName;
			itemTypeAttrName << DPopItemsTypePrefix << i;

			opts.append_attribute(itemTypeAttrName.str().c_str()) = this->GetNumPoppedItems(PopItem::E_Type(i));
		}

		//Save number of lightnings
		opts.append_attribute(DNumLightningsAttr) = this->GetNumLightnings();

		//Save number of chainlocks
		opts.append_attribute(DNumChainLocksAttr) = this->GetNumChainLocks();

		//Save number of ice cubes
		opts.append_attribute(DNumIceCubesAttr) = this->GetNumIceCubes();

		//Save time spent in game
		opts.append_attribute(DTimeSpentInGameLAttr) = u32(this->GetTimeSpentInGame() & 0xFFFFFFFF);
		opts.append_attribute(DTimeSpentInGameHAttr) = u32(((this->GetTimeSpentInGame()) >> 32) & 0xFFFFFFFF);
	}


	//Save audio settings
	{
		pugi::xml_node opts = doc.append_child();
		opts.set_name(DAudioTag);

		//Save sound muted setting
		opts.append_attribute(DSoundMutedAttr) = this->IsSoundMuted();

		//Save music muted setting
		opts.append_attribute(DMusicMutedAttr) = this->IsMusicMuted();
	}


	//TODO:




	class PrefsWriter : public pugi::xml_writer{
		ting::fs::FSFile f;
	public:
		PrefsWriter(const std::string& fileName) :
				f(fileName)
		{
			f.Open(ting::fs::File::CREATE);
		}

		//override
		void write(const void *data, size_t size){
			ting::Array<ting::u8> buf(size);
			memcpy(buf.Begin(), data, size);
			f.Write(buf);
		}
	} writer(this->prefsFileName);

    doc.print(writer);
}
