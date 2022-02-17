#pragma once

#include <cmath>
#include <string>

namespace displib {
	class V3D {
		public:
		float x, y, z;

		V3D();
		V3D(float x_, float y_, float z_);

		V3D add(V3D v);
		static V3D add(const V3D& a, const V3D& b);

		V3D sub(V3D v);
		static V3D sub(const V3D& a, const V3D& b);

		V3D mult(float f);
		static V3D mult(const V3D& v, float f);

		V3D div(float f);
		static V3D div(const V3D& v, float f);

		float dot(V3D v);
		static float dot(V3D a, V3D b);

		V3D cross(V3D v);
		static V3D cross(V3D a, V3D b);

		V3D normal();
		static V3D normal(const V3D& v);

		float mag();

		std::string toStr();

		V3D operator +(const V3D& v);
		V3D& operator +=(const V3D& v);
		V3D operator -(const V3D& v);
		V3D& operator -=(const V3D& v);
		V3D operator *(float f);
		V3D& operator *=(float f);
		V3D operator /(float f);
		V3D& operator /=(float f);
		V3D& operator =(const V3D& v);
	};
}