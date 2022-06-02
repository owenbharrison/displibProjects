#include "vector/V2D.h"

namespace displib {
#pragma once
	class Maths {
		private:
		Maths();

		public:
		static const float PI, TAU, E;

		static float lerp(float t, float a, float b);

		static float map(float t, float a, float b, float c, float d);

		static float clamp(float t, float a, float b);

		static int sign(float f);

		static float snapTo(float a, float b);

		static float random();

		static float random(float f);

		static float random(float a, float b);

		static float* lineLineIntersection(V2D a, V2D b, V2D c, V2D d);

		static bool lineLineIntersect(V2D a, V2D b, V2D c, V2D d);

		static V2D* lineLineIntersectPt(V2D a, V2D b, V2D c, V2D d);
	};
}