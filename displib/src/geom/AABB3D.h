#include "../maths/vector/float3.h"

namespace displib {
#pragma once
	class AABB3D {
		public:
		float3 min, max;

		AABB3D();

		AABB3D(float minx, float miny, float minz, float maxx, float maxy, float maxz);

		AABB3D(float3 min_, float3 max_);

		//is pt in bounding box
		bool containsPt(float3 pt);

		//rectangle bool overlap
		bool overlapAABB(AABB3D aabb);
	};
}