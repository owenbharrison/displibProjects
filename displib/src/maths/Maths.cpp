#include "Maths.h"
#include <math.h>
#include <time.h>

namespace displib {
	//simple constants
	const float Maths::PI=3.1415926f;
	const float Maths::TAU=Maths::PI*2;
	const float Maths::E=2.7182818f;

	//this is a private constructor, dont use.
	Maths::Maths() {}
	
	//returns value t "percent(0-1)" between a and b.
	float Maths::lerp(float t, float a, float b) {
		return a+(b-a)*t;
	}

	//maps one range to another, linearly.
	float Maths::map(float x, float a, float b, float c, float d) {
		return (x-a)*(d-c)/(b-a)+c;
	}

	//makes t inside the range a and b.
	float Maths::clamp(float t, float a, float b) {
		if (t<a)return a;
		if (t>b)return b;
		return t;
	}

	//gives the integer sign of a number.
	int Maths::sign(float f) {
		return f/abs(f);
	}

	//snaps a to nearest stepped b value.
	float Maths::snapTo(float a, float b) {
		return roundf(a/b)*b;
	}

	/*returns t and u values for intersection tests.
	remember to clear mem after this!*/
	float* Maths::lineLineIntersection(V2D a, V2D b, V2D c, V2D d) {
		float q=(a.x-b.x)*(c.y-d.y)-(a.y-b.y)*(c.x-d.x);
		return new float[2]{
			((a.x-c.x)*(c.y-d.y)-(a.y-c.y)*(c.x-d.x))/q,
			((b.x-a.x)*(a.y-c.y)-(b.y-a.y)*(a.x-c.x))/q
		};
	}

	//returns whether the lines [a,b] and [c,d] are intersecting.
	bool Maths::lineLineIntersect(V2D a, V2D b, V2D c, V2D d) {
		float* tu=Maths::lineLineIntersection(a, b, c, d);
		bool ix=tu[0]>=0&&tu[0]<=1&&tu[1]>=0&&tu[1]<=1;
		delete[] tu;
		return ix;
	}

	//returns random val in range(0, 1).
	float Maths::random() {
		return rand()/32767.0f;
	}

	//returns random val in range(0, f).
	float Maths::random(float f) {
		return Maths::random()*f;
	}

	//returns random val in range(a, b).
	float Maths::random(float a, float b) {
		return a+Maths::random(b-a);
	}
}