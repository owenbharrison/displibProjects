#include <cmath>

namespace displib {
#pragma once
	struct float2 {
		float x, y;

		float2();

		float2(float f);

		float2(float x_, float y_);

		//vector check
		bool operator==(float2 v);

		//vector set
		void operator=(float2 v);
		void operator=(float f);

		//vector add then set
		void operator+=(float2 v);
		void operator+=(float f);

		//vector subtract then set
		void operator-=(float2 v);
		void operator-=(float f);

		//vector multiply then set
		void operator*=(float2 v);
		void operator*=(float f);

		//vector divide then set
		void operator/=(float2 v);
		void operator/=(float f);
	};

	//vector addition
	float2 operator+(float2 a, float2 b);
	float2 operator+(float2 v, float f);
	float2 operator+(float f, float2 v);

	//vector subtraction
	float2 operator-(float2 a, float2 b);
	float2 operator-(float2 v, float f);
	float2 operator-(float f, float2 v);

	//vector multiplication
	float2 operator*(float2 a, float2 b);
	float2 operator*(float2 v, float f);
	float2 operator*(float f, float2 v);

	//vector division
	float2 operator/(float2 a, float2 b);
	float2 operator/(float2 v, float f);
	float2 operator/(float f, float2 v);

	//dot prod of two vecs.
	float dot(float2 a, float2 b);

	//scalar magnitude of vec.
	float length(float2 v);

	//divide all components by length, thus mag is 1.
	float2 normalize(float2 v);
}