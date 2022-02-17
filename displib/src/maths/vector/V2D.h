#pragma once

#include <string>

namespace displib {
	class V2D {
		public:
		float x, y;

		V2D();
		V2D(float x_, float y_);

		V2D add(V2D v);
		static V2D add(const V2D& a, const V2D& b);

		V2D sub(V2D v);
		static V2D sub(const V2D& a, const V2D& b);

		V2D mult(float f);
		static V2D mult(const V2D& v, float f);

		V2D div(float f);
		static V2D div(const V2D& v, float f);

		float dot(V2D v);
		static float dot(V2D a, V2D b);

		V2D normal();
		static V2D normal(const V2D& v);

		float heading();

		float mag();

		static V2D fromAngle(float f);

		std::string toStr();

		V2D operator +(const V2D& v);
		V2D& operator +=(const V2D& v);
		V2D operator -(const V2D& v);
		V2D& operator -=(const V2D& v);
		V2D operator *(float f);
		V2D& operator *=(float f);
		V2D operator /(float f);
		V2D& operator /=(float f);
		V2D& operator =(const V2D& v);
	};
}