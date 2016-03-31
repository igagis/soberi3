/* 
 * File:   ResFont.hpp
 * Author: ivan
 *
 * Created on September 19, 2010, 12:46 PM
 */

#pragma once

#include <string>

#include "../ResMan/ResMan.hpp"

#include "../MyMaemoFW/TexFont.hpp"



class ResFont : public resman::Resource{
	friend class resman::ResMan;

	TexFont font;

	ResFont(ting::fs::File& fi, const std::wstring& chars, unsigned size, unsigned outline) :
			font(fi, chars.c_str(), size, outline)
	{}

public:
	~ResFont()throw(){}

	inline float RenderString(const tride::Matr4f& matrix, const std::string& s, const tride::Vec4f& color)const{
		return this->font.RenderString(matrix, s, color);
	}

	inline float RenderString(const tride::Matr4f& matrix, const std::string& s)const{
		return this->font.RenderString(matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const wchar_t* s)const{
		return this->font.RenderString(shader, matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const std::wstring& s)const{
		return this->font.RenderString(shader, matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const char* s)const{
		return this->font.RenderString(shader, matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const std::string& s)const{
		return this->font.RenderString(shader, matrix, s);
	}

	inline TexFont::BoundingBox StringBoundingBox(const wchar_t* s)const{
		return this->font.StringBoundingBox(s);
	}

	inline TexFont::BoundingBox StringBoundingBox(const char* s)const{
		return this->font.StringBoundingBox(s);
	}

	inline TexFont::BoundingBox StringBoundingBox(const std::string& s)const{
		return this->font.StringBoundingBox(s);
	}

	inline float FontSize()const{
		return this->font.FontSize();
	}

	inline const TexFont::BoundingBox& FontBoundingBox()const{
		return this->font.FontBoundingBox();
	}

private:
	static ting::Ref<ResFont> Load(pugi::xml_node el, ting::fs::File &fi);
};

