#include "../maths/vector/V3D.h"

namespace displib {
#pragma once
	class AABB3D {
		public:
		V3D min, max;

		AABB3D();

		AABB3D(float minx, float miny, float minz, float maxx, float maxy, float maxz);

		AABB3D(V3D min_, V3D max_);

		bool containsPt(V3D pt);

		bool overlapAABB(AABB3D aabb);
	};
}