#include "vector/float2.h"

namespace displib {
#pragma once
	class Maths {
		private:
		Maths();

		public:
		static const float PI, TAU, E;

		//returns value t "percent(0-1)" between a and b.
		static float lerp(float t, float a, float b);

		//maps one range to another, linearly.
		static float map(float t, float a, float b, float c, float d);

		//makes t inside the range a and b.
		static float clamp(float t, float a, float b);

		//gives the integer sign of a number.
		static int sign(float f);

		//snaps a to nearest stepped b value.
		static float snapTo(float a, float b);

		//returns random val in range(0, 1).
		static float random();

		//returns random val in range(0, f).
		static float random(float f);

		//returns random val in range(a, b).
		static float random(float a, float b);

		//t and u values for intersection tests
		static void lineLineIntersection(float2 a, float2 b, float2 c, float2 d, float* t, float* u);

		//returns whether the lines [a,b] and [c,d] are intersecting.
		static bool lineLineIntersect(float2 a, float2 b, float2 c, float2 d);
	};
}