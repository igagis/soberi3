#include <ting/debug.hpp>

#include "MyMaemoFW/Shader.hpp"

#include "Sprite.hpp"



using namespace ting;



void Sprite::Scale(tride::Vec2f scale){
	for(unsigned i = 0; i < this->verts.Size(); ++i){
		this->verts[i].x *= scale.x;
		this->verts[i].y *= scale.y;
	}
}



void Sprite::RenderModulating(const tride::Matr4f& matrix, const tride::Vec4f& color)const{
	ModulatingColoringTexturingShader &s = ModulatingColoringTexturingShader::Inst();
	s.UseProgram();

	s.SetColor(color);
	s.DisableColorPointer();

	s.SetMatrix(matrix);

	ASS(this->tex)->Bind();

	s.EnablePositionPointer();
	s.EnableTexCoordPointer();
	s.SetPositionPointer(this->verts.Begin());
	s.SetTexCoordPointer(this->texCoords.Begin());

	s.DrawArrays(GL_TRIANGLE_FAN, 4);
}



void Sprite::RenderSaturating(const tride::Matr4f& matrix, const tride::Vec4f& color)const{
	SaturatingColoringTexturingShader &s = SaturatingColoringTexturingShader::Inst();
	s.UseProgram();

	s.SetColor(color);
	s.DisableColorPointer();

	s.SetMatrix(matrix);

	ASS(this->tex)->Bind();

	s.EnablePositionPointer();
	s.EnableTexCoordPointer();
	s.SetPositionPointer(this->verts.Begin());
	s.SetTexCoordPointer(this->texCoords.Begin());

	s.DrawArrays(GL_TRIANGLE_FAN, 4);
}



void Sprite::Render(const tride::Matr4f& matrix)const{
	SimpleTexturingShader &s = SimpleTexturingShader::Inst();
	s.UseProgram();

	s.SetMatrix(matrix);

	ASS(this->tex)->Bind();

	s.EnablePositionPointer();
	s.EnableTexCoordPointer();
	s.SetPositionPointer(this->verts.Begin());
	s.SetTexCoordPointer(this->texCoords.Begin());
	ASSERT(this->verts.Size() == this->texCoords.Size())
	s.DrawArrays(GL_TRIANGLE_FAN, this->verts.Size());
}

