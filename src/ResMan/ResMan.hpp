// (c) Ivan Gagis
// e-mail: igagis@gmail.com
// Version: 5

// Description:
//          Resources manager class

#pragma once

#include <map>
#include <sstream>

#include <ting/Ref.hpp>
#include <ting/Singleton.hpp>
#include <ting/fs/FSFile.hpp>

#include <pugixml/pugixml.hpp>

#include "../MyMaemoFW/ZipFile.hpp"

#define M_ENABLE_RESOURCE_LOG
#ifdef M_ENABLE_RESOURCE_LOG
#define M_RESOURCE_LOG(x) LOG(x)
#else
#define M_RESOURCE_LOG(x)
#endif


namespace resman{

//forward declarations
class ResMan;


//base class for all resources
class Resource : public ting::RefCounted{
	friend class ResMan;

	struct StringComparator{
		bool operator()(const std::string& s1, const std::string& s2)const{
			return s1.compare(s2) < 0;
		}
	};
	typedef std::map<std::string, ting::WeakRef<Resource>, StringComparator> T_ResMap;
	typedef T_ResMap::iterator T_ResMapIter;

	//ResMapRC = Resource Map RefCounted
	class ResMapRC : public RefCounted{
	public:
		~ResMapRC()throw(){}
		
		T_ResMap rm;

		static inline ting::Ref<ResMapRC> New(){
			return ting::Ref<ResMapRC>(new ResMapRC());
		}
	};

	ting::WeakRef<ResMapRC> rm;
	T_ResMapIter it;

protected:
	//this can only be used as a base class
	Resource();
public:
	virtual ~Resource()throw();
};



//
//
//      Resource manager class
//
//
class ResMan : public ting::Singleton<ResMan>{
    friend class Resource;

	typedef Resource::T_ResMap T_ResMap;
	typedef T_ResMap::iterator T_ResMapIter;
	
	ting::Ref<Resource::ResMapRC> resMap;

	class ResPackEntry{
	public:
		ting::Ptr<ting::fs::File> fi;
		ting::Ptr<pugi::xml_document> resScript;
	};

	typedef std::vector<ResPackEntry> T_ResPackList;
	typedef T_ResPackList::iterator T_ResPackIter;

	//list of mounted resource packs
	T_ResPackList resPacks;


	class FindInScriptRet{
	public:
		FindInScriptRet(ResPackEntry* resPack, const pugi::xml_node element) :
				rp(resPack),
				e(element)
		{}

		ResPackEntry* rp;
		const pugi::xml_node e;

		inline bool IsValid()const{
			return !e.empty() && rp != 0;
		}
	};

	FindInScriptRet FindResourceInScript(const std::string& resName);

	template <class T> ting::Ref<T> FindResourceInResMap(const std::string& resName);

	//Add resource to resources map
	void AddResource(ting::Ref<Resource> res, const std::string& resName);

	
public:
	inline ResMan() :
			resMap(Resource::ResMapRC::New())
	{}

	virtual ~ResMan(){}

	void MountResPack(ting::Ptr<ting::fs::File> fi);

	inline void MountResPackDir(const std::string& dir){
		ting::Ptr<ting::fs::FSFile> fi(new ting::fs::FSFile());
		fi->SetRootDir(dir);
		this->MountResPack(fi);
	}

	inline void MountResPackZip(const std::string& zipFile){
		ting::Ptr<ZipFile> fi(new ZipFile(zipFile));
		fi->OpenZipFile();
		this->MountResPack(fi);
	}

	template <class T> ting::Ref<T> Load(const std::string& resName);
};



template <class T> ting::Ref<T> ResMan::FindResourceInResMap(const std::string& resName){
	T_ResMapIter i = this->resMap->rm.find(resName);
	if(i != this->resMap->rm.end()){
		ting::Ref<Resource> r((*i).second);
		ASSERT(r.DynamicCast<T>().IsValid())
		return r.StaticCast<T>();
	}
	return ting::Ref<T>();//no resource found with given name, return invalid reference
}



template <class T> ting::Ref<T> ResMan::Load(const std::string& resName){
	M_RESOURCE_LOG(<< "ResMan::Load(): enter" << std::endl)
	if(ting::Ref<T> r = this->FindResourceInResMap<T>(resName)){
		M_RESOURCE_LOG(<< "ResManHGE::Load(): resource found in map" << std::endl)
		return r;
	}

	M_RESOURCE_LOG(<< "ResMan::Load(): searching for resource in script..." << std::endl)
	ting::fs::File* fi DEBUG_CODE(= 0);
	pugi::xml_node el;
	{
		FindInScriptRet ret = this->FindResourceInScript(resName);
		if(!ret.IsValid()){
			M_RESOURCE_LOG(<< "ResMan::Load(): there is no resource with given name, throwing exception" << std::endl)
			std::stringstream ss;
			ss << "ResMan::Load(): there is no resource with given name: " << resName;
			throw ting::Exc(ss.str().c_str());
		}
		fi = ASS(ret.rp)->fi.operator->();
		el = ret.e;
	}
	ASSERT(fi)

	M_RESOURCE_LOG(<< "ResMan::Load(): resource found in script" << std::endl)

	ting::Ref<T> resource = T::Load(el, *fi);

	this->AddResource(resource, resName);

	M_RESOURCE_LOG(<< "ResMan::LoadTexture(): exit" << std::endl)
	return resource;
}



}//~namespace
