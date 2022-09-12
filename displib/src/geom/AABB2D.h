#include "../maths/vector/float2.h"
#include "../io/Raster.h"

namespace displib {
#pragma once
	class AABB2D {
		public:
		float2 min, max;

		AABB2D();

		AABB2D(float minx, float miny, float maxx, float maxy);

		AABB2D(float2 min_, float2 max_);

		//is pt in bounding box
		bool containsPt(float2 pt);

		//rectangle bool overlap
		bool overlapAABB(AABB2D aabb);

		//draw to raster.
		void render(Raster& gfx);
	};
}