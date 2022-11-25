#include "float2.h"

namespace displib {
	float2::float2() : x(0), y(0) {}

	float2::float2(float f) : x(f), y(f) {}

	float2::float2(float x_, float y_) : x(x_), y(y_) {}

	bool float2::operator==(float2 v) { return x==v.x&&y==v.y; }

	void float2::operator=(float2 v) { x=v.x; y=v.y; }
	void float2::operator=(float f) { x=y=f; }

	//ugly overloads :C
	float2 operator+(float2 a, float2 b) { return float2(a.x+b.x, a.y+b.y); }
	float2 operator+(float2 v, float f) { return v+float2(f); }
	float2 operator+(float f, float2 v) { return float2(f)+v; }
	void float2::operator+=(float2 v) { *this=*this+v; }
	void float2::operator+=(float f) { *this=*this+f; }

	float2 operator-(float2 a, float2 b) { return float2(a.x-b.x, a.y-b.y); }
	float2 operator-(float2 v, float f) { return v-float2(f); }
	float2 operator-(float f, float2 v) { return float2(f)-v; }
	void float2::operator-=(float2 v) { *this=*this-v; }
	void float2::operator-=(float f) { *this=*this-f; }

	float2 operator*(float2 a, float2 b) { return float2(a.x*b.x, a.y*b.y); }
	float2 operator*(float2 v, float f) { return v*float2(f); }
	float2 operator*(float f, float2 v) { return float2(f)*v; }
	void float2::operator*=(float2 v) { *this=*this*v; }
	void float2::operator*=(float f) { *this=*this*f; }

	float2 operator/(float2 a, float2 b) { return float2(a.x/b.x, a.y/b.y); }
	float2 operator/(float2 v, float f) { return v/float2(f); }
	float2 operator/(float f, float2 v) { return float2(f)/v; }
	void float2::operator/=(float2 v) { *this=*this/v; }
	void float2::operator/=(float f) { *this=*this/f; }

	float dot(float2 a, float2 b) {
		float2 c=a*b;
		return c.x+c.y;
	}

	float length(float2 v) {
		return sqrtf(dot(v, v));
	}

	float2 normalize(float2 v) {
		float l=length(v);
		return l==0?v:v/l;
	}
}