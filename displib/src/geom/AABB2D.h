#include "../maths/vector/V2D.h"
#include "../io/Raster.h"

namespace displib {
#pragma once
	class AABB2D {
		public:
		V2D min, max;

		AABB2D();

		AABB2D(float minx, float miny, float maxx, float maxy);

		AABB2D(V2D min_, V2D max_);

		bool containsPt(V2D pt);

		bool overlapAABB(AABB2D aabb);

		void render(Raster& gfx);
	};
}