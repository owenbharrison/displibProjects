#include "Maths.h"
#include <cmath>
#include <time.h>

namespace displib {
	const float Maths::PI=3.1415926f;
	const float Maths::TAU=Maths::PI*2.0f;

	Maths::Maths() {}

	float Maths::lerp(float t, float a, float b) {
		return t*(b-a)+a;
	}

	float Maths::map(float x, float a, float b, float c, float d) {
		return (x-a)*(d-c)/(b-a)+c;
	}

	float Maths::clamp(float t, float a, float b) {
		if (t<a)return a;
		if (t>b)return b;
		return t;
	}

	float Maths::sign(float f) {
		return f/abs(f);
	}

	float Maths::snapTo(float a, float b) {
		return roundf(a/b)*b;
	}

	float Maths::fastInvSqrt(float n) {
		const float threehalfs=1.5F;
		float y=n;
		long i=*(long*)&y;
		i=0x5f3759df-(i>>1);
		y=*(float*)&i;
		y=y*(threehalfs-((n*0.5F)*y*y));
		return y;
	}

	float* Maths::lineLineIntersection(V2D a, V2D b, V2D c, V2D d) {
		float q=(a.x-b.x)*(c.y-d.y)-(a.y-b.y)*(c.x-d.x);
		float f[2]={0.0f};
		f[0]=((a.x-c.x)*(c.y-d.y)-(a.y-c.y)*(c.x-d.x))/q;
		f[1]=((b.x-a.x)*(a.y-c.y)-(b.y-a.y)*(a.x-c.x))/q;
		return f;
	}

	bool Maths::lineLineIntersect(V2D a, V2D b, V2D c, V2D d) {
		float* f=Maths::lineLineIntersection(a, b, c, d);
		return f[0]>=0.0&&f[0]<=1.0&&f[1]>=0.0&&f[1]<=1.0;
	}

	/*to use Maths::lineLineIntersectPt:
	V2D* ix=Maths::lineLineIntersectPt(a, b, c, d);
	if(ix!=nullptr){
		//there is an intersection
		//V2D pt = *ix;
	}
	else{
		//no intersect
	}
	*/
	V2D* Maths::lineLineIntersectPt(V2D a, V2D b, V2D c, V2D d) {
		float* f=Maths::lineLineIntersection(a, b, c, d);
		if (f[0]>=0.0&&f[0]<=1.0&&f[1]>=0.0&&f[1]<=1.0) {
			V2D pt=a+(b-a)*f[0];
			return &pt;
		}
		return nullptr;
	}

	float Maths::random() {
		return rand()/32767.0f;
	}

	float Maths::random(float f) {
		return Maths::random()*f;
	}

	float Maths::random(float a, float b) {
		return a+Maths::random(b-a);
	}
}