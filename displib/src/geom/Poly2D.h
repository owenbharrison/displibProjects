#include "../maths/Maths.h"
#include "../maths/vector/V2D.h"
#include "../geom/AABB2D.h"

namespace displib {
#pragma once
	class Poly2D {
		public:
		V2D pos;
		float angle;
		int numPts;
		V2D* model;//angle, dist
		V2D* points;

		Poly2D();//this just makes a completely empty poly2d

		Poly2D(V2D pos_, V2D* pts, int num);

		Poly2D(V2D pos_, float rad, int num);

		void updatePoints();

		AABB2D getAABB();

		bool containsPt(V2D pt);

		void render(Raster rst);
	};
}