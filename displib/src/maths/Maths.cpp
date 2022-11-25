#include "Maths.h"

namespace displib {
	//simple constants
	const float Maths::PI=3.1415926f;
	const float Maths::TAU=Maths::PI*2;
	const float Maths::E=2.7182818f;

	Maths::Maths() {}
	
	float Maths::lerp(float t, float a, float b) {
		return a+(b-a)*t;
	}

	float Maths::invLerp(float v, float a, float b) {
		return (v-a)/(b-a);
	}

	float Maths::map(float x, float a, float b, float c, float d) {
		return (x-a)*(d-c)/(b-a)+c;
	}

	float Maths::clamp(float t, float a, float b) {
		if (t<a) return a;
		if (t>b) return b;
		return t;
	}

	int Maths::sign(float f) {
		return f/abs(f);
	}

	float Maths::snapTo(float a, float b) {
		return roundf(a/b)*b;
	}

	void Maths::lineLineIntersection(float2 a, float2 b, float2 c, float2 d, float* t, float* u) {
		float q=(a.x-b.x)*(c.y-d.y)-(a.y-b.y)*(c.x-d.x);
		*t=((a.x-c.x)*(c.y-d.y)-(a.y-c.y)*(c.x-d.x))/q;
		*u=((b.x-a.x)*(a.y-c.y)-(b.y-a.y)*(a.x-c.x))/q;
	}

	bool Maths::lineLineIntersect(float2 a, float2 b, float2 c, float2 d) {
		float t, u;
		Maths::lineLineIntersection(a, b, c, d, &t, &u);
		return t>=0&&t<=1&&u>=0&&u<=1;
	}

	float Maths::random() {
		return rand()/(float)RAND_MAX;
	}

	float Maths::random(float f) {
		return Maths::random()*f;
	}

	float Maths::random(float a, float b) {
		return a+Maths::random(b-a);
	}
}