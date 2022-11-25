#include "float3.h"

namespace displib {
	float3::float3() : x(0), y(0), z(0) {}

	float3::float3(float f) : x(f), y(f), z(f) {}

	float3::float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	bool float3::operator==(float3 v) { return x==v.x&&y==v.y&&z==v.z; }

	void float3::operator=(float3 v) { x=v.x; y=v.y; z=v.z; }
	void float3::operator=(float f) { x=y=z=f; }

	//ugly overloads :C
	float3 operator+(float3 a, float3 b) { return float3(a.x+b.x, a.y+b.y, a.z+b.z); }
	float3 operator+(float3 v, float f) { return v+float3(f); }
	float3 operator+(float f, float3 v) { return float3(f)+v; }
	void float3::operator+=(float3 v) { *this=*this+v; }
	void float3::operator+=(float f) { *this=*this+f; }

	float3 operator-(float3 a, float3 b) { return float3(a.x-b.x, a.y-b.y, a.z-b.z); }
	float3 operator-(float3 v, float f) { return v-float3(f); }
	float3 operator-(float f, float3 v) { return float3(f)-v; }
	void float3::operator-=(float3 v) { *this=*this-v; }
	void float3::operator-=(float f) { *this=*this-f; }

	float3 operator*(float3 a, float3 b) { return float3(a.x*b.x, a.y*b.y, a.z*b.z); }
	float3 operator*(float3 v, float f) { return v*float3(f); }
	float3 operator*(float f, float3 v) { return float3(f)*v; };
	void float3::operator*=(float3 v) { *this=*this*v; }
	void float3::operator*=(float f) { *this=*this*f; }

	float3 operator/(float3 a, float3 b) { return float3(a.x/b.x, a.y/b.y, a.z/b.z); }
	float3 operator/(float3 v, float f) { return v/float3(f); }
	float3 operator/(float f, float3 v) { return float3(f)/v; }
	void float3::operator/=(float3 v) { *this=*this/v; }
	void float3::operator/=(float f) { *this=*this/f; }

	float dot(float3 a, float3 b) {
		float3 c=a*b;
		return c.x+c.y+c.z;
	}

	float3 cross(float3 a, float3 b) {
		return float3(
			a.y*b.z-a.z*b.y,
			a.z*b.x-a.x*b.z,
			a.x*b.y-a.y*b.x
		);
	}

	float length(float3 v) {
		return sqrtf(dot(v, v));
	}

	float3 normalize(float3 v) {
		float l=length(v);
		return l==0?v:v/l;
	}
}
