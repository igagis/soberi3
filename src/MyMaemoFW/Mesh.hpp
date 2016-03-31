// Author: Ivan Gagis <igagis@gmail.com>
// Version: 2.0

#pragma once

#include <tride/Vector3.hpp>

#include <ting/Array.hpp>
#include <ting/types.hpp>


class Mesh{
public:
	class Face{
		ting::u16 v[3];
	public:
		inline Face(){}
		inline Face(ting::u16 val){
			this->v[0] = val;
			this->v[1] = val;
			this->v[2] = val;
		}
		inline Face(ting::u16 v1, ting::u16 v2, ting::u16 v3){
			this->v[0] = v1;
			this->v[1] = v2;
			this->v[2] = v3;
		}

		inline ting::u16& operator[](unsigned i)throw(){
			ASSERT(i < 3)
			return this->v[i];
		}

		inline const ting::u16& operator[](unsigned i)const throw(){
			ASSERT(i < 3)
			return this->v[i];
		}

		inline Face& operator=(const Face& f)throw(){
			this->v[0] = f.v[0];
			this->v[1] = f.v[1];
			this->v[2] = f.v[2];
			return (*this);
		}
	};
	STATIC_ASSERT(sizeof(Face) == (sizeof(ting::u16) * 3))

protected:
	ting::Array<tride::Vec3f> v;//vertices
	ting::Array<Face> f;//faces

	ting::Array<tride::Vec2f> t;//texture coordinates, may be invalid
public:

	const ting::Array<tride::Vec3f>& Vertices()const{
		return this->v;
	}

	const ting::Array<Face>& Faces()const{
		return this->f;
	}

	const ting::Array<tride::Vec2f>& TexCoords()const{
		return this->t;
	}


	/**
	 * @brief Construct mesh.
	 * @param numberOfVertices - number of vertices in this mesh (i.e. size of the vertices array).
	 * @param numberOfFaces - number of faces in this mesh (i.e. size of faces array).
	 */
	inline Mesh(ting::u16 numberOfVertices, unsigned numberOfFaces) :
			v(numberOfVertices),
			f(numberOfFaces)
	{}

	virtual ~Mesh(){}
};

