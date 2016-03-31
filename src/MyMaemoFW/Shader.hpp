/* 
 * File:   Shader.hpp
 * Author: ivan
 *
 * Created on February 16, 2010, 9:02 PM
 */

#pragma once

#include <GLES2/gl2.h>

#include <tride/Vector3.hpp>

#include <ting/Singleton.hpp>

#include "Mesh.hpp"

class Shader{
protected:
	GLuint vertexShader;
	GLuint fragmentShader;
	GLuint program;

	GLuint positionAttr;

	GLuint matrixUniform;
public:
	Shader();

	~Shader();

	inline void UseProgram(){
		ASSERT(this->program != 0)
		glUseProgram(this->program);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void SetMatrix(const tride::Matr4f &m){
		glUniformMatrix4fv(this->matrixUniform, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&m));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void SetPositionPointer(const tride::Vec3f *p){
		ASSERT(p)
		glVertexAttribPointer(this->positionAttr, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLfloat*>(p));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void SetPositionPointer(const tride::Vec2f *p){
		ASSERT(p)
		glVertexAttribPointer(this->positionAttr, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLfloat*>(p));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void EnablePositionPointer(){
		glEnableVertexAttribArray(this->positionAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DisablePositionPointer(){
		glDisableVertexAttribArray(this->positionAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DrawArrays(GLenum mode, unsigned numElements){
		glDrawArrays(mode, 0, numElements);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DrawMesh(GLenum mode, const Mesh& mesh){
		this->EnablePositionPointer();
		this->SetPositionPointer(mesh.Vertices().Begin());
		STATIC_ASSERT(sizeof(Mesh::Face) == sizeof(ting::u16) * 3)

		glDrawElements(mode, mesh.Faces().Size() * 3, GL_UNSIGNED_SHORT, mesh.Faces().Begin());
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	void DrawQuad(GLenum mode = GL_TRIANGLE_FAN);

	void DrawQuad01(GLenum mode = GL_TRIANGLE_FAN);

protected:
	//return true if not compiled
	static bool CheckForCompileErrors(GLuint shader);

	//return true if not linked
	static bool CheckForLinkErrors(GLuint program);
};//~class Shader



class ColoringShader : virtual public Shader{

	//no copying
	ColoringShader(const ColoringShader& sh){
		ASSERT(false)
	}

protected:
	GLuint vertexColorAttr;

	inline ColoringShader(){}
public:
	inline void SetColor(tride::Vec3f color){
		STATIC_ASSERT(sizeof(color) == sizeof(GLfloat) * 3)
		glVertexAttrib3fv(this->vertexColorAttr, reinterpret_cast<GLfloat*>(&color));
		ASSERT(glGetError() == GL_NO_ERROR)
    }

	inline void SetColor(tride::Vec3f color, float alpha){
		STATIC_ASSERT(sizeof(color) == sizeof(GLfloat) * 3)
		GLfloat clr[4] = {
			color.x,
			color.y,
			color.z,
			alpha
		};
		glVertexAttrib4fv(this->vertexColorAttr, clr);
		ASSERT(glGetError() == GL_NO_ERROR)
    }

	inline void SetColor(tride::Vec4f color){
		STATIC_ASSERT(sizeof(color) == sizeof(GLfloat) * 4)
		glVertexAttrib4fv(this->vertexColorAttr, reinterpret_cast<GLfloat*>(&color));
		ASSERT(glGetError() == GL_NO_ERROR)
    }

	inline void SetColorPointer(tride::Vec3f *p){
		ASSERT(p)
		glVertexAttribPointer(this->vertexColorAttr, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLfloat*>(p));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void SetColorPointer(tride::Vec4f *p){
		ASSERT(p)
		glVertexAttribPointer(this->vertexColorAttr, 4, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLfloat*>(p));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void EnableColorPointer(){
		glEnableVertexAttribArray(this->vertexColorAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DisableColorPointer(){
		glDisableVertexAttribArray(this->vertexColorAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}
};//~class ColoringShader



class SimpleColoringShader :
		public ColoringShader,
		public ting::Singleton<SimpleColoringShader>
{
public:
	SimpleColoringShader();
};



class TexturingShader : virtual public Shader{

	//no copying
	TexturingShader(const TexturingShader&){
		ASSERT(false)
	}
	
protected:
	GLuint texCoordAttr;

	GLuint texNumberUniform;

	inline TexturingShader(){}

public:
	inline void SetTextureNumber(unsigned i){
		glUniform1i(this->texNumberUniform, i);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void SetTexCoordPointer(const tride::Vec2f *p){
		ASSERT(p)
		glVertexAttribPointer(this->texCoordAttr, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<const GLfloat*>(p));
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void EnableTexCoordPointer(){
		glEnableVertexAttribArray(this->texCoordAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DisableTexCoordPointer(){
		glDisableVertexAttribArray(this->texCoordAttr);
		ASSERT(glGetError() == GL_NO_ERROR)
	}

	inline void DrawMesh(GLenum mode, const Mesh& mesh){
		if(mesh.TexCoords().IsValid()){
			this->EnableTexCoordPointer();
			this->SetTexCoordPointer(mesh.TexCoords().Begin());
		}else{
			this->DisableTexCoordPointer();
		}

		this->Shader::DrawMesh(mode, mesh);
	}

	void DrawQuad(GLenum mode = GL_TRIANGLE_FAN);
	
	void DrawQuad01(GLenum mode = GL_TRIANGLE_FAN);
};//~class TexturingShader



class SimpleTexturingShader :
		public TexturingShader,
		public ting::Singleton<SimpleTexturingShader>
{
public:
	SimpleTexturingShader();
	
};



class ColoringTexturingShader :
		public TexturingShader,
		public ColoringShader,
		public ting::Singleton<ColoringTexturingShader>
{
public:
	ColoringTexturingShader();

	inline void DrawMesh(GLenum mode, const Mesh& mesh){
		this->DisableColorPointer();

		this->TexturingShader::DrawMesh(mode, mesh);
	}
};



class ModulatingColoringTexturingShader :
		public TexturingShader,
		public ColoringShader,
		public ting::Singleton<ModulatingColoringTexturingShader>
{
public:
	ModulatingColoringTexturingShader();

	inline void DrawMesh(GLenum mode, const Mesh& mesh){
		this->DisableColorPointer();

		this->TexturingShader::DrawMesh(mode, mesh);
	}
};



class SaturatingColoringTexturingShader :
		public TexturingShader,
		public ColoringShader,
		public ting::Singleton<SaturatingColoringTexturingShader>
{
public:
	SaturatingColoringTexturingShader();

	inline void DrawMesh(GLenum mode, const Mesh& mesh){
		this->DisableColorPointer();

		this->TexturingShader::DrawMesh(mode, mesh);
	}
};

