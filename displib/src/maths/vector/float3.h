#include <cmath>

namespace displib {
#pragma once
	struct float3 {
		float x, y, z;

		float3();

		float3(float f);

		float3(float x_, float y_, float z_);

		//vector check
		bool operator==(float3 v);

		//vector set
		void operator=(float3 v);
		void operator=(float f);

		//vector add then set
		void operator+=(float3 v);
		void operator+=(float f);

		//vector subtract then set
		void operator-=(float3 v);
		void operator-=(float f);

		//vector multiply then set
		void operator*=(float3 v);
		void operator*=(float f);

		//vector divide then set
		void operator/=(float3 v);
		void operator/=(float f);
	};

	//vector addition
	float3 operator+(float3 a, float3 b);
	float3 operator+(float3 v, float f);
	float3 operator+(float f, float3 v);

	//vector subtraction
	float3 operator-(float3 a, float3 b);
	float3 operator-(float3 v, float f);
	float3 operator-(float f, float3 v);

	//vector multiplication
	float3 operator*(float3 a, float3 b);
	float3 operator*(float3 v, float f);
	float3 operator*(float f, float3 v);

	//vector division
	float3 operator/(float3 a, float3 b);
	float3 operator/(float3 v, float f);
	float3 operator/(float f, float3 v);

	//dot prod of two vecs.
	float dot(float3 a, float3 b);

	//cross prod of two vecs.
	float3 cross(float3 a, float3 b);

	//scalar magnitude of vec.
	float length(float3 v);

	//divide all components by length, thus mag is 1.
	float3 normalize(float3 v);
}