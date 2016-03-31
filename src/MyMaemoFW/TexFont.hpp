// Author: Ivan Gagis <igagis@gmail.com>

// File description:
//          Texture font class

#pragma once

#include <map>
#include <sstream>
#include <stdexcept>

#include <tride/Vector3.hpp>

#include <ting/types.hpp>
#include <ting/Exc.hpp>
#include <ting/fs/File.hpp>

#include <GLES2/gl2.h>
//#include <GL/gl.h>

#include "GLTexture.hpp"
#include "Shader.hpp"


class TexFont{
	struct Glyph{
		ting::StaticBuffer<tride::Vec2f, 4> verts;
		ting::StaticBuffer<tride::Vec2f, 4> texCoords;

		float advance;
	};

	GLTexture tex;

	typedef std::map<wchar_t, Glyph> T_GlyphsMap;
	typedef T_GlyphsMap::iterator T_GlyphsIter;
	T_GlyphsMap glyphs;

	float fontSize;

public:
	class BoundingBox{
	public:
		float left;
		float right;
		float top;
		float bottom;

		inline float Width()const{
			return ASSCOND(this->left + this->right, >= 0);
		}

		inline float Height()const{
			return ASSCOND(this->top + this->bottom, >= 0);
		}

		inline void SetToZero(){
			this->left = 0;
			this->right = 0;
			this->top = 0;
			this->bottom = 0;
		}
	};

private:
	//Bounding box holds the dimensions of the largest loaded glyph.
	BoundingBox boundingBox;

public:
	TexFont(){}

	TexFont(ting::fs::File& fi, const wchar_t* chars, unsigned size, unsigned outline = 0){
		this->Load(fi, chars, size, outline);
	}

	void Load(ting::fs::File& fi, const wchar_t* chars, unsigned size, unsigned outline = 0);

	inline float FontSize()const{
		return this->fontSize;
	}

	//renders the string, returns resulting string length
	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const wchar_t* s)const{
		return this->RenderStringInternal<wchar_t>(shader, matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const std::wstring& s)const{
		return this->RenderStringInternal<wchar_t>(shader, matrix, s.c_str());
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const char* s)const{
		return this->RenderStringInternal<char>(shader, matrix, s);
	}

	inline float RenderString(const tride::Matr4f& matrix, const std::string& s)const{
		SimpleTexturingShader& shader = SimpleTexturingShader::Inst();
		return this->RenderString(shader, matrix, s);
	}

	inline float RenderString(const tride::Matr4f& matrix, const std::string& s, const tride::Vec4f& color)const{
		ModulatingColoringTexturingShader& shader = ModulatingColoringTexturingShader::Inst();
		shader.DisableColorPointer();
		shader.SetColor(color);
		return this->RenderString(shader, matrix, s);
	}

	inline float RenderString(TexturingShader& shader, const tride::Matr4f& matrix, const std::string& s)const{
		return this->RenderStringInternal<char>(shader, matrix, s.c_str());
	}

	inline const BoundingBox& FontBoundingBox()const{
		return this->boundingBox;
	}

	inline float StringWidth(const wchar_t* s)const{
		return this->StringWidthInternal<wchar_t>(s);
	}

	inline BoundingBox StringBoundingBox(const wchar_t* s)const{
		return this->StringBoundingBoxInternal<wchar_t>(s);
	}

	inline BoundingBox StringBoundingBox(const char* s)const{
		return this->StringBoundingBoxInternal<char>(s);
	}

	inline BoundingBox StringBoundingBox(const std::string& s)const{
		return this->StringBoundingBoxInternal<char>(s.c_str());
	}

	DEBUG_CODE( void RenderTex(const tride::Matr4f& matrix)const; )

private:

	template <class T> float StringWidthInternal(const T* s)const{
		float ret = 0;

		try{
			for(; *s != 0; ++s){
				const Glyph& g = this->glyphs.at(wchar_t(*s));
				ret += g.advance;
			}
		}catch(std::out_of_range& e){
			std::stringstream ss;
			ss << "TexFont::StringWidthInternal(): Character is not loaded, scan code = 0x" << std::hex << *s;
			throw ting::Exc(ss.str().c_str());
		}

		return ret;
	}

	template <class T> BoundingBox StringBoundingBoxInternal(const T* s)const{
		BoundingBox ret;

		if(*s == 0){
			ret.SetToZero();
			return ret;
		}

		float curAdvance = 0;

		//init ret with bounding box of the first glyph
		{
			const Glyph& g = this->glyphs.at(wchar_t(*s));
			ret.left = g.verts[0].x;
			ret.right = g.verts[2].x;
			ret.top = g.verts[1].y;
			ret.bottom = g.verts[0].y;
			curAdvance += g.advance;
			++s;
		}

		try{
			for(; *s != 0; ++s){
				const Glyph& g = this->glyphs.at(wchar_t(*s));

				if(g.verts[1].y > ret.top){
					ret.top = g.verts[1].y;
				}
				
				if(g.verts[0].y < ret.bottom){
					ret.bottom = - g.verts[0].y;
				}

				if(curAdvance + g.verts[0].x < ret.left){
					ret.left = -(curAdvance + g.verts[0].x);
				}

				if(curAdvance + g.verts[2].x > ret.right){
					ret.right = curAdvance + g.verts[2].x;
				}

				curAdvance += g.advance;
			}
		}catch(std::out_of_range& e){
			std::stringstream ss;
			ss << "TexFont::StringBoundingLineInternal(): Character is not loaded, scan code = 0x" << std::hex << *s;
			throw ting::Exc(ss.str().c_str());
		}

		ASSERT(ret.Width() >= 0)
		ASSERT(ret.Height() >= 0)
		return ret;
	}

	inline float RenderGlyphInternal(TexturingShader& shader, const tride::Matr4f& matrix, wchar_t ch)const{
		const Glyph& g = this->glyphs.at(ch);

		shader.SetMatrix(matrix);

		shader.SetPositionPointer(g.verts.Begin());
		shader.SetTexCoordPointer(g.texCoords.Begin());

		shader.DrawArrays(GL_TRIANGLE_FAN, g.verts.Size());

		return g.advance;
	}


	
	template <class T> float RenderStringInternal(TexturingShader& shader, const tride::Matr4f& matrix, const T* s)const{
		shader.UseProgram();
		shader.EnablePositionPointer();
		shader.EnableTexCoordPointer();

		this->tex.Bind();

		float ret = 0;

		tride::Matr4f matr(matrix);

		try{
			for(; *s != 0; ++s){
				float advance = this->RenderGlyphInternal(shader, matr, wchar_t(*s));
				ret += advance;
				matr.Translate(advance, 0);
			}
		}catch(std::out_of_range& e){
			std::stringstream ss;
			ss << "TexFont::RenderStringInternal(): Character is not loaded, scan code = 0x" << std::hex << *s;
			throw ting::Exc(ss.str().c_str());
		}

		return ret;
	}
};//~class TexFont
